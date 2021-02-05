//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleG.h"

namespace PetriEngine {
  bool ReductionRuleG::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      if (!remove_loops) return false;
      bool continueReductions = false;
      for (uint32_t t = 0; t < reducer->parent->numberOfTransitions(); ++t) {
          if (reducer->hasTimedout()) return false;
          Transition &trans = reducer->parent->_transitions[t];
          if (trans.skip) continue;
          if (trans.inhib) continue;
          if (trans.pre.size() < trans.post.size()) continue;
          if (!remove_loops && trans.pre.size() == 0) continue;

          auto postit = trans.post.begin();
          auto preit = trans.pre.begin();

          bool ok = true;
          while (true) {
              if (preit == trans.pre.end() && postit == trans.post.end())
                  break;
              if (preit == trans.pre.end()) {
                  ok = false;
                  break;
              }
              if (preit->inhib || reducer->parent->_places[preit->place].inhib) {
                  ok = false;
                  break;
              }
              if (postit != trans.post.end() && preit->place == postit->place) {
                  if (!remove_consumers && preit->weight != postit->weight) {
                      ok = false;
                      break;
                  }
                  if ((placeInQuery[preit->place] > 0 && preit->weight != postit->weight) ||
                      (placeInQuery[preit->place] == 0 && preit->weight < postit->weight)) {
                      ok = false;
                      break;
                  }
                  ++preit;
                  ++postit;
              } else if (postit == trans.post.end() || preit->place < postit->place) {
                  if (placeInQuery[preit->place] > 0 || !remove_consumers) {
                      ok = false;
                      break;
                  }
                  ++preit;
              } else {
                  // could not match a post with a pre
                  ok = false;
                  break;
              }
          }
          if (ok) {
              for (preit = trans.pre.begin(); preit != trans.pre.end(); ++preit) {
                  if (preit->inhib || reducer->parent->_places[preit->place].inhib) {
                      ok = false;
                      break;
                  }
              }
          }

          if (!ok) continue;
          // TODO: ++_ruleG;
          skipTransition(t);
      }
      assert(reducer->consistent());
      return continueReductions;
  }
}