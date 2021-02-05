//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleI.h"

namespace PetriEngine {
  bool ReductionRuleI::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      bool reduced = false;
      if (remove_loops) {
          std::vector<uint32_t> wtrans;
          std::vector<bool> tseen(reducer->parent->numberOfTransitions(), false);

          for (uint32_t p = 0; p < reducer->parent->numberOfPlaces(); ++p) {
              if (reducer->hasTimedout()) return false;
              if (placeInQuery[p] > 0) {
                  const Place &place = reducer->parent->_places[p];
                  for (auto t : place.consumers) {
                      if (!tseen[t]) {
                          wtrans.push_back(t);
                          tseen[t] = true;
                      }
                  }
                  for (auto t : place.producers) {
                      if (!tseen[t]) {
                          wtrans.push_back(t);
                          tseen[t] = true;
                      }
                  }
              }
          }

          std::vector<bool> pseen(reducer->parent->numberOfPlaces(), false);
          while (!wtrans.empty()) {
              if (reducer->hasTimedout()) return false;
              auto t = wtrans.back();
              wtrans.pop_back();
              const Transition &trans = reducer->parent->_transitions[t];
              for (const Arc &arc : trans.pre) {
                  const Place &place = reducer->parent->_places[arc.place];
                  if (arc.inhib) {
                      for (auto pt : place.consumers) {
                          if (!tseen[pt]) {
                              Transition &trans = reducer->parent->_transitions[pt];
                              auto it = trans.post.begin();
                              for (; it != trans.post.end(); ++it)
                                  if (it->place >= arc.place) break;
                              if (it != trans.post.end() && it->place == arc.place) {
                                  auto it2 = trans.pre.begin();
                                  for (; it2 != trans.pre.end(); ++it2)
                                      if (it2->place >= arc.place || it2->inhib) break;
                                  if (it->weight <= it2->weight) continue;
                              }
                              tseen[pt] = true;
                              wtrans.push_back(pt);
                          }
                      }
                  } else {
                      for (auto pt : place.producers) {
                          if (!tseen[pt]) {
                              Transition &trans = reducer->parent->_transitions[pt];
                              auto it = trans.pre.begin();
                              for (; it != trans.pre.end(); ++it)
                                  if (it->place >= arc.place) break;

                              if (it != trans.pre.end() && it->place == arc.place && !it->inhib) {
                                  auto it2 = trans.post.begin();
                                  for (; it2 != trans.post.end(); ++it2)
                                      if (it2->place >= arc.place) break;
                                  if (it->weight >= it2->weight) continue;
                              }

                              tseen[pt] = true;
                              wtrans.push_back(pt);
                          }
                      }

                      for (auto pt : place.consumers) {
                          if (!tseen[pt] && (!remove_consumers || placeInQuery[arc.place] > 0)) {
                              tseen[pt] = true;
                              wtrans.push_back(pt);
                          }
                      }
                  }
                  pseen[arc.place] = true;
              }
          }

          for (size_t t = 0; t < reducer->parent->numberOfTransitions(); ++t) {
              if (!tseen[t] && !reducer->parent->_transitions[t].skip) {
                  skipTransition(t);
                  reduced = true;
              }
          }

          for (size_t p = 0; p < reducer->parent->numberOfPlaces(); ++p) {
              if (!pseen[p] && !reducer->parent->_places[p].skip && placeInQuery[p] == 0) {
                  assert(placeInQuery[p] == 0);
                  skipPlace(p);
                  reduced = true;
              }
          }

          if (reduced) {
              // TODO: ++_ruleI;
          }
      } else {
          const size_t numberofplaces = reducer->parent->numberOfPlaces();
          for (uint32_t p = 0; p < numberofplaces; ++p) {
              if (reducer->hasTimedout()) return false;
              Place &place = reducer->parent->_places[p];
              if (place.skip) continue;
              if (place.inhib) continue;
              if (placeInQuery[p] > 0) continue;
              if (!place.consumers.empty()) continue;

              // TODO: ++_ruleI;
              reduced = true;

              std::vector<uint32_t> torem;
              if (remove_consumers) {
                  for (auto &t : place.producers) {
                      auto &trans = reducer->parent->_transitions[t];
                      if (trans.post.size() != 1) // place will be removed later
                          continue;
                      bool ok = true;
                      for (auto &a : trans.pre) {
                          if (placeInQuery[a.place] > 0) {
                              ok = false;
                          }
                      }
                      if (ok) torem.push_back(t);
                  }
              }
              skipPlace(p);
              for (auto t : torem)
                  skipTransition(t);
              assert(reducer->consistent());
          }
      }

      return reduced;
  }
}