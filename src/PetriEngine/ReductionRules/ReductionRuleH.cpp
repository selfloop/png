//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleH.h"

namespace PetriEngine {
  bool ReductionRuleH::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      if (reconstructTrace)
          return false; // we don't know where in the loop the tokens are needed
      auto transok = [this](uint32_t t) -> uint32_t {
        auto &trans = parent->_transitions[t];
        if (_tflags[t] != 0)
            return _tflags[t];
        _tflags[t] = 1;
        if (trans.inhib ||
            trans.pre.size() != 1 ||
            trans.post.size() != 1) {
            return 2;
        }

        auto p1 = trans.pre[0].place;
        auto p2 = trans.post[0].place;

        // we actually do not need weights to be 1 here.
        // there is a special case when the places are always "inputting"
        // and "outputting" with a GCD that is equal to the weight of the
        // specific transition.
        // Ie, the place always have a number of tokens (disregarding
        // initial tokens) that is dividable with the transition weight

        if (trans.pre[0].weight != 1 ||
            trans.post[0].weight != 1 ||
            p1 == p2 ||
            parent->_places[p1].inhib ||
            parent->_places[p2].inhib) {
            return 2;
        }
        return 1;
      };

      auto removeLoop = [this, placeInQuery](std::vector<uint32_t> &loop) -> bool {
        size_t i = 0;
        for (; i < loop.size(); ++i)
            if (loop[i] == loop.back())
                break;

        assert(_tflags[loop.back()] == 1);
        if (i == loop.size() - 1)
            return false;

        auto p1 = parent->_transitions[loop[i]].pre[0].place;
        bool removed = false;

        for (size_t j = i + 1; j < loop.size() - 1; ++j) {
            if (hasTimedout())
                return removed;
            auto p2 = parent->_transitions[loop[j]].pre[0].place;
            if (placeInQuery[p2] > 0 || placeInQuery[p1] > 0) {
                p1 = p2;
                continue;
            }
            if (p1 == p2) {
                continue;
            }
            removed = true;
            // TODO: ++_ruleH;
            skipTransition(loop[j - 1]);
            auto &place1 = parent->_places[p1];
            auto &place2 = parent->_places[p2];

            {

                for (auto p2it : place2.consumers) {
                    auto &t = parent->_transitions[p2it];
                    auto arc = getInArc(p2, t);
                    assert(arc != t.pre.end());
                    assert(arc->place == p2);
                    auto a = *arc;
                    a.place = p1;
                    auto dest = std::lower_bound(t.pre.begin(), t.pre.end(), a);
                    if (dest == t.pre.end() || dest->place != p1) {
                        t.pre.insert(dest, a);
                        auto lb = std::lower_bound(place1.consumers.begin(), place1.consumers.end(), p2it);
                        place1.consumers.insert(lb, p2it);
                    } else {
                        dest->weight += a.weight;
                    }
                    consistent();
                }
            }

            {
                auto p2it = place2.producers.begin();

                for (; p2it != place2.producers.end(); ++p2it) {
                    auto &t = parent->_transitions[*p2it];
                    Arc a = *getOutArc(t, p2);
                    a.place = p1;
                    auto dest = std::lower_bound(t.post.begin(), t.post.end(), a);
                    if (dest == t.post.end() || dest->place != p1) {
                        t.post.insert(dest, a);
                        auto lb = std::lower_bound(place1.producers.begin(), place1.producers.end(), *p2it);
                        place1.producers.insert(lb, *p2it);
                    } else {
                        dest->weight += a.weight;
                    }
                    consistent();
                }
            }
            parent->initialMarking[p1] += parent->initialMarking[p2];
            skipPlace(p2);
            assert(placeInQuery[p2] == 0);
        }
        return removed;
      };

      bool continueReductions = false;
      for (uint32_t t = 0; t < parent->numberOfTransitions(); ++t) {
          if (hasTimedout())
              return continueReductions;
          _tflags.resize(parent->_transitions.size(), 0);
          std::fill(_tflags.begin(), _tflags.end(), 0);
          std::vector<uint32_t> stack;
          {
              if (_tflags[t] != 0) continue;
              auto &trans = parent->_transitions[t];
              if (trans.skip) continue;
              _tflags[t] = transok(t);
              if (_tflags[t] != 1) continue;
              stack.push_back(t);
          }
          bool outer = true;
          while (!stack.empty() && outer) {
              if (hasTimedout())
                  return continueReductions;
              auto it = stack.back();
              auto post = parent->_transitions[it].post[0].place;
              bool found = false;
              for (auto &nt : parent->_places[post].consumers) {
                  if (hasTimedout())
                      return continueReductions;
                  auto &nexttrans = parent->_transitions[nt];
                  if (nt == it || nexttrans.skip)
                      continue; // handled elsewhere
                  if (_tflags[nt] == 1 && stack.size() > 1) {
                      stack.push_back(nt);
                      bool found = removeLoop(stack);
                      continueReductions |= found;

                      if (found) {
                          outer = false;
                          break;
                      } else {
                          stack.pop_back();
                      }
                  } else if (_tflags[nt] == 0) {
                      _tflags[nt] = transok(nt);
                      if (_tflags[nt] == 2) {
                          continue;
                      } else {
                          assert(_tflags[nt] == 1);
                          stack.push_back(nt);
                          found = true;
                          break;
                      }
                  } else {
                      continue;
                  }
              }
              if (!found && outer) {
                  _tflags[it] = 2;
                  stack.pop_back();
              }
          }
      }
      return continueReductions;
  }
}