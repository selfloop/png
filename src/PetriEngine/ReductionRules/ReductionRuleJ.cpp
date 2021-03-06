//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleJ.h"

namespace PetriEngine {
  bool ReductionRuleJ::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      bool continueReductions = false;
      for (uint32_t t = 0; t < reducer->parent->numberOfTransitions(); ++t) {
          if (reducer->hasTimedout())
              return continueReductions;

          if (reducer->parent->_transitions[t].skip ||
              reducer->parent->_transitions[t].inhib ||
              reducer->parent->_transitions[t].pre.size() != 1)
              continue;
          auto p = reducer->parent->_transitions[t].pre[0].place;
          if (placeInQuery[p] > 0) {
              continue; // can be relaxed
          }
          if (reducer->parent->initialMarking[p] > 0) {
              continue; // can be relaxed
          }
          const Place &place = reducer->parent->_places[p];
          if (place.skip) continue;
          if (place.inhib) continue;
          if (place.consumers.empty()) continue;
          if (place.producers.empty()) continue;

          // check that prod and cons are not overlapping
          const auto presize = place.producers.size(); // can be relaxed >= 2
          const auto postsize = place.consumers.size(); // can be relaxed >= 2
          bool ok = true;
          for (size_t i = 0; i < postsize; ++i) {   // this can be done smarter than a quadratic loop!
              for (size_t j = 0; j < presize; ++j) {
                  ok &= place.consumers[i] != place.producers[j];
              }
          }
          if (!ok) continue;
          // check that post of consumer is not messing with query or inhib
          // if either all pre or all post are query-free, we are ok.
          bool inquery = false;
          for (auto t : place.consumers) {
              Transition &trans = reducer->parent->_transitions[t];
              if (trans.pre.size() == 1) // can be relaxed
              {
                  // check that weights match
                  // can be relaxed
                  ok &= trans.pre[0].weight == 1;
                  ok &= !trans.pre[0].inhib;
              } else {
                  ok = false;
                  break;
              }
              for (auto &pp : trans.post) {
                  ok &= !reducer->parent->_places[pp.place].inhib;
                  inquery |= placeInQuery[pp.place] > 0;
                  ok &= pp.weight == 1; // can be relaxed
              }
              if (!ok)
                  break;
          }
          if (!ok) continue;
          // check that pre of producing do not mess with query or inhib
          for (auto &t : place.producers) {
              Transition &trans = reducer->parent->_transitions[t];
              for (const auto &arc : trans.post) {
                  ok &= !inquery || placeInQuery[arc.place] == 0;
                  ok &= !reducer->parent->_places[arc.place].inhib;
              }
          }
          if (!ok) continue;
          // TODO: ++_ruleJ;
          continueReductions = true;
          // otherwise we can skip the place by merging up the two transitions
          // constructing 4 new transitions, one for each combination.
          // In the binary case, we want to achieve the following four transitions
          // post[n] = pre[n] + post[n]
          // pre[0] = pre[0] + post[1]
          // pre[1] = pre[1] + post[0]

          // start by copying out the post of each of the posts
          Place pp = place;
          skipPlace(p);
          std::vector<std::vector<Arc>> posts;
          std::vector<Transition> pres;

          for (auto t : pp.consumers)
              posts.push_back(reducer->parent->_transitions[t].post);

          for (auto t : pp.producers)
              pres.push_back(reducer->parent->_transitions[t]);

          // remove old transitions, we will create new ones
          for (auto t : pp.consumers)
              skipTransition(t);

          for (auto t : pp.producers)
              skipTransition(t);

          // compute all permutations
          for (auto &trans : pres) {
              for (auto &postset : posts) {
                  auto id = reducer->parent->_transitions.size();
                  if (!reducer->_skipped_trans.empty())
                      id = reducer->_skipped_trans.back();
                  else {
                      continue;
                      reducer->parent->_transitions.emplace_back();
                  }
                  reducer->parent->_transitions[id] = trans;
                  auto &target = reducer->parent->_transitions[id];
                  for (auto &arc : postset)
                      target.addPostArc(arc);

                  // add to places
                  if (reducer->_skipped_trans.empty())
                      reducer->parent->_transitionnames[newTransName()] = id;

                  for (auto &arc : target.pre)
                      reducer->parent->_places[arc.place].addConsumer(id);
                  for (auto &arc : target.post)
                      reducer->parent->_places[arc.place].addProducer(id);
                  if (!reducer->_skipped_trans.empty()) {
                      --reducer->_removedTransitions; // recycling
                      reducer->_skipped_trans.pop_back();
                  }
                  reducer->parent->_transitions[id].skip = false;
                  reducer->parent->_transitions[id].inhib = false;
                  reducer->consistent();
              }
          }
          reducer->consistent();
      }
      return continueReductions;
  }

  std::string ReductionRuleJ::newTransName() {
      auto prefix = "CT";
      auto tmp = prefix + std::to_string(_tnameid);
      while (reducer->parent->_transitionnames.count(tmp) >= 1) {
          ++_tnameid;
          tmp = prefix + std::to_string(_tnameid);
      }
      ++_tnameid;
      return tmp;
  }
}