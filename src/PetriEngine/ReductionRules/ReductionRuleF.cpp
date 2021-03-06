//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleF.h"

namespace PetriEngine {
  bool ReductionRuleF::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      bool continueReductions = false;
      const size_t numberofplaces = reducer->parent->numberOfPlaces();
      for (uint32_t p = 0; p < numberofplaces; ++p) {
          if (reducer->hasTimedout()) return false;
          Place &place = reducer->parent->_places[p];
          if (place.skip) continue;
          if (place.inhib) continue;
          if (place.producers.size() < place.consumers.size()) continue;
          if (placeInQuery[p] != 0) continue;

          bool ok = true;
          for (uint cons : place.consumers) {
              Transition &t = getTransition(cons);
              auto w = reducer->getInArc(p, t)->weight;
              if (w > reducer->parent->initialMarking[p]) {
                  ok = false;
                  break;
              } else {
                  auto it = reducer->getOutArc(t, p);
                  if (it == t.post.end() ||
                      it->place != p ||
                      it->weight < w) {
                      ok = false;
                      break;
                  }
              }
          }

          if (!ok) continue;

          // TODO: ++_ruleF;

          if ((numberofplaces - reducer->_removedPlaces) > 1) {
              if (reducer->reconstructTrace) {
                  for (auto t : place.consumers) {
                      std::string tname = getTransitionName(t);
                      const ArcIter arc = reducer->getInArc(p, getTransition(t));
                      reducer->_extraconsume[tname].emplace_back(reducer->getPlaceName(p), arc->weight);
                  }
              }
              skipPlace(p);
              continueReductions = true;
          }

      }
      assert(reducer->consistent());
      return continueReductions;
  }
}