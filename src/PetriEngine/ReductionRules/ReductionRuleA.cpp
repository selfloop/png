//
// Created by hamburger on 04.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleA.h"

namespace PetriEngine {
  bool ReductionRuleA::reduce(uint32_t *placeInQuery) {
      // Rule A  - find transition t that has exactly one place in pre and post and remove one of the places (and t)
      bool continueReductions = false;
      const size_t numberoftransitions = parent->numberOfTransitions();
      for (uint32_t t = 0; t < numberoftransitions; t++) {
          if (hasTimedout()) return false;
          Transition &trans = getTransition(t);

          // we have already removed
          if (trans.skip) continue;

          // A2. we have more/less than one arc in pre or post
          // checked first to avoid out-of-bounds when looking up indexes.
          if (trans.pre.size() != 1) continue;

          uint32_t pPre = trans.pre[0].place;

          // A2. Check that pPre goes only to t
          if (parent->_places[pPre].consumers.size() != 1) continue;

          // A3. We have weight of more than one on input
          // and is empty on output (should not happen).
          auto w = trans.pre[0].weight;
          bool ok = true;
          for (auto t : parent->_places[pPre].producers) {
              if ((getOutArc(parent->_transitions[t], trans.pre[0].place)->weight % w) != 0) {
                  ok = false;
                  break;
              }
          }
          if (!ok) continue;

          // A4. Do inhibitor check, neither T, pPre or pPost can be involved with any inhibitor
          if (parent->_places[pPre].inhib || trans.inhib) continue;

          // A5. dont mess with query!
          if (placeInQuery[pPre] > 0) continue;
          // check A1, A4 and A5 for post
          for (auto &pPost : trans.post) {
              if (parent->_places[pPost.place].inhib || pPre == pPost.place || placeInQuery[pPost.place] > 0) {
                  ok = false;
                  break;
              }
          }
          if (!ok) continue;

          continueReductions = true;
          // TODO: _ruleA++;

          // here we need to remember when a token is created in pPre (some
          // transition with an output in P is fired), t is fired instantly!.
          if (reconstructTrace) {
              Place &pre = parent->_places[pPre];
              std::string tname = getTransitionName(t);
              for (size_t pp : pre.producers) {
                  std::string prefire = getTransitionName(pp);
                  _postfire[prefire].push_back(tname);
              }
              _extraconsume[tname].emplace_back(getPlaceName(pPre), w);
              for (size_t i = 0; i < parent->initMarking()[pPre]; ++i) {
                  _initfire.push_back(tname);
              }
          }

          for (auto &pPost : trans.post) {
              // UA2. move the token for the initial marking, makes things simpler.
              parent->initialMarking[pPost.place] += ((parent->initialMarking[pPre] / w) * pPost.weight);
          }
          parent->initialMarking[pPre] = 0;

          // Remove transition t and the place that has no tokens in m0
          // UA1. remove transition
          auto toMove = trans.post;
          skipTransition(t);

          // UA2. update arcs
          for (auto &_t : parent->_places[pPre].producers) {
              assert(_t != t);
              // move output-arcs to post.
              Transition &src = getTransition(_t);
              auto source = *getOutArc(src, pPre);
              for (auto &pPost : toMove) {
                  Arc a;
                  a.place = pPost.place;
                  a.weight = (source.weight / w) * pPost.weight;
                  assert(a.weight > 0);
                  a.inhib = false;
                  auto dest = std::lower_bound(src.post.begin(), src.post.end(), a);
                  if (dest == src.post.end() || dest->place != pPost.place) {
                      dest = src.post.insert(dest, a);
                      auto &prod = parent->_places[pPost.place].producers;
                      auto lb = std::lower_bound(prod.begin(), prod.end(), _t);
                      prod.insert(lb, _t);
                  } else {
                      dest->weight += ((source.weight / w) * pPost.weight);
                  }
                  assert(dest->weight > 0);
              }
          }
          // UA1. remove place
          skipPlace(pPre);
      } // end of Rule A main for-loop
      return continueReductions;
  }
}