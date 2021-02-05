//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleD.h"

namespace PetriEngine {
  bool ReductionRuleD::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      // Rule D - two transitions with the same pre and post and same inhibitor arcs
      // This does not alter the trace.
      bool continueReductions = false;
      _tflags.resize(reducer->parent->_transitions.size(), 0);
      std::fill(_tflags.begin(), _tflags.end(), 0);
      bool has_empty_trans = false;
      for (size_t t = 0; t < reducer->parent->_transitions.size(); ++t) {
          auto &trans = reducer->parent->_transitions[t];
          if (!trans.skip && trans.pre.size() == 0 && trans.post.size() == 0) {
              if (has_empty_trans) {
                  // TODO: ++_ruleD;
                  skipTransition(t);
              }
              has_empty_trans = true;
          }

      }
      for (auto &op : reducer->parent->_places)
          for (size_t outer = 0; outer < op.consumers.size(); ++outer) {
              auto touter = op.consumers[outer];
              if (reducer->hasTimedout()) return false;
              if (_tflags[touter] != 0) continue;
              _tflags[touter] = 1;
              Transition &tout = getTransition(touter);
              if (tout.skip) continue;

              // D2. No inhibitors
              if (tout.inhib) continue;

              for (size_t inner = outer + 1; inner < op.consumers.size(); ++inner) {
                  auto tinner = op.consumers[inner];
                  Transition &tin = getTransition(tinner);
                  if (tin.skip || tout.skip) continue;

                  // D2. No inhibitors
                  if (tin.inhib) continue;

                  for (size_t swp = 0; swp < 2; ++swp) {
                      if (reducer->hasTimedout()) return false;

                      if (tin.skip || tout.skip) break;

                      uint t1 = touter;
                      uint t2 = tinner;
                      if (swp == 1) std::swap(t1, t2);

                      // D1. not same transition
                      assert(t1 != t2);

                      Transition &trans1 = getTransition(t1);
                      Transition &trans2 = getTransition(t2);

                      // From D3, and D4 we have that pre and post-sets are the same
                      if (trans1.post.size() != trans2.post.size()) break;
                      if (trans1.pre.size() != trans2.pre.size()) break;

                      int ok = 0;
                      uint mult = std::numeric_limits<uint>::max();
                      // D4. postsets must match
                      for (int i = trans1.post.size() - 1; i >= 0; --i) {
                          Arc &arc = trans1.post[i];
                          Arc &arc2 = trans2.post[i];
                          if (arc2.place != arc.place) {
                              ok = 2;
                              break;
                          }

                          if (mult == std::numeric_limits<uint>::max()) {
                              if (arc2.weight < arc.weight || (arc2.weight % arc.weight) != 0) {
                                  ok = 1;
                                  break;
                              } else {
                                  mult = arc2.weight / arc.weight;
                              }
                          } else if (arc2.weight != arc.weight * mult) {
                              ok = 2;
                              break;
                          }
                      }

                      if (ok == 2) break;
                      else if (ok == 1) continue;

                      // D3. Presets must match
                      for (int i = trans1.pre.size() - 1; i >= 0; --i) {
                          Arc &arc = trans1.pre[i];
                          Arc &arc2 = trans2.pre[i];
                          if (arc2.place != arc.place) {
                              ok = 2;
                              break;
                          }

                          if (mult == std::numeric_limits<uint>::max()) {
                              if (arc2.weight < arc.weight || (arc2.weight % arc.weight) != 0) {
                                  ok = 1;
                                  break;
                              } else {
                                  mult = arc2.weight / arc.weight;
                              }
                          } else if (arc2.weight != arc.weight * mult) {
                              ok = 2;
                              break;
                          }
                      }

                      if (ok == 2) break;
                      else if (ok == 1) continue;

                      // UD1. Remove transition t2
                      continueReductions = true;
                      // TODO: _ruleD++;
                      skipTransition(t2);
                      _tflags[touter] = 0;
                      break; // break the swap loop
                  }
              }
          } // end of main for loop for rule D
      assert(reducer->consistent());
      return continueReductions;
  }
}