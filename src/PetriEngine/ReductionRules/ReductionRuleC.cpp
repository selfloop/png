//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleC.h"

namespace PetriEngine {
  bool ReductionRuleC::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      // Rule C - Places with same input and output-transitions which a modulo each other
      bool continueReductions = false;

      _pflags.resize(reducer->parent->_places.size(), 0);
      std::fill(_pflags.begin(), _pflags.end(), 0);

      for (uint32_t touter = 0; touter < reducer->parent->numberOfTransitions(); ++touter)
          for (size_t outer = 0; outer < reducer->parent->_transitions[touter].post.size(); ++outer) {
              auto pouter = reducer->parent->_transitions[touter].post[outer].place;
              if (_pflags[pouter] > 0) continue;
              _pflags[pouter] = 1;
              if (reducer->hasTimedout()) return false;
              if (reducer->parent->_places[pouter].skip) continue;

              // C4. No inhib
              if (reducer->parent->_places[pouter].inhib) continue;

              for (size_t inner = outer + 1; inner < reducer->parent->_transitions[touter].post.size(); ++inner) {
                  auto pinner = reducer->parent->_transitions[touter].post[inner].place;
                  if (reducer->parent->_places[pinner].skip) continue;

                  // C4. No inhib
                  if (reducer->parent->_places[pinner].inhib) continue;

                  for (size_t swp = 0; swp < 2; ++swp) {
                      if (reducer->hasTimedout()) return false;
                      if (reducer->parent->_places[pinner].skip ||
                          reducer->parent->_places[pouter].skip)
                          break;

                      uint p1 = pouter;
                      uint p2 = pinner;

                      if (swp == 1) std::swap(p1, p2);

                      Place &place1 = reducer->parent->_places[p1];

                      // C1. Not same place
                      if (p1 == p2) break;

                      // C5. Dont mess with query
                      if (placeInQuery[p2] > 0)
                          continue;

                      Place &place2 = reducer->parent->_places[p2];

                      // C2, C3. Consumer and producer-sets must match
                      if (place1.consumers.size() < place2.consumers.size() ||
                          place1.producers.size() > place2.producers.size())
                          break;

                      long double mult = 1;

                      // C8. Consumers must match with weights
                      int ok = 0;
                      size_t j = 0;
                      for (size_t i = 0; i < place2.consumers.size(); ++i) {
                          while (j < place1.consumers.size() && place1.consumers[j] < place2.consumers[i]) ++j;
                          if (place1.consumers.size() <= j || place1.consumers[j] != place2.consumers[i]) {
                              ok = 2;
                              break;
                          }

                          Transition &trans = getTransition(place1.consumers[j]);
                          auto a1 = reducer->getInArc(p1, trans);
                          auto a2 = reducer->getInArc(p2, trans);
                          assert(a1 != trans.pre.end());
                          assert(a2 != trans.pre.end());
                          mult = std::max(mult, ((long double) a2->weight) / ((long double) a1->weight));
                      }

                      if (ok == 2) break;

                      // C6. We do not care about excess markings in p2.
                      if (mult != std::numeric_limits<long double>::max() &&
                          (((long double) reducer->parent->initialMarking[p1]) * mult)
                              > ((long double) reducer->parent->initialMarking[p2])) {
                          continue;
                      }


                      // C7. Producers must match with weights
                      j = 0;
                      for (size_t i = 0; i < place1.producers.size(); ++i) {
                          while (j < place2.producers.size() && place2.producers[j] < place1.producers[i]) ++j;
                          if (j == place2.producers.size() || place1.producers[i] != place2.producers[j]) {
                              ok = 2;
                              break;
                          }

                          Transition &trans = getTransition(place1.producers[i]);
                          auto a1 = reducer->getOutArc(trans, p1);
                          auto a2 = reducer->getOutArc(trans, p2);
                          assert(a1 != trans.post.end());
                          assert(a2 != trans.post.end());

                          if (((long double) a1->weight) * mult > ((long double) a2->weight)) {
                              ok = 1;
                              break;
                          }
                      }

                      if (ok == 2) break;
                      else if (ok == 1) continue;

                      reducer->parent->initialMarking[p2] = 0;

                      if (reducer->reconstructTrace) {
                          for (auto t : place2.consumers) {
                              std::string tname = getTransitionName(t);
                              const ArcIter arc = reducer->getInArc(p2, getTransition(t));
                              reducer->_extraconsume[tname].emplace_back(reducer->getPlaceName(p2), arc->weight);
                          }
                      }

                      continueReductions = true;
                      // TODO: _ruleC++;
                      // UC1. Remove p2
                      skipPlace(p2);
                      _pflags[pouter] = 0;
                      break;
                  }
              }
          }
      assert(reducer->consistent());
      return continueReductions;
  }
}