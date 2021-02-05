//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleE.h"

namespace PetriEngine {
  bool ReductionRuleE::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      bool continueReductions = false;
      const size_t numberofplaces = reducer->parent->numberOfPlaces();
      for (uint32_t p = 0; p < numberofplaces; ++p) {
          if (reducer->hasTimedout()) return false;
          Place &place = reducer->parent->_places[p];
          if (place.skip) continue;
          if (place.inhib) continue;
          if (place.producers.size() > place.consumers.size()) continue;

          std::set<uint32_t> notenabled;
          bool ok = true;
          for (uint cons : place.consumers) {
              Transition &t = getTransition(cons);
              auto in = reducer->getInArc(p, t);
              if (in->weight <= reducer->parent->initialMarking[p]) {
                  auto out = reducer->getOutArc(t, p);
                  if (out == t.post.end() || out->place != p || out->weight >= in->weight) {
                      ok = false;
                      break;
                  }
              } else {
                  notenabled.insert(cons);
              }
          }

          if (!ok || notenabled.empty()) continue;

          for (uint prod : place.producers) {
              if (notenabled.count(prod) == 0) {
                  ok = false;
                  break;
              }
              // check that producing arcs originate from transition also
              // consuming. If so, we know it will never fire.
              Transition &t = getTransition(prod);
              ArcIter it = reducer->getInArc(p, t);
              if (it == t.pre.end()) {
                  ok = false;
                  break;
              }
          }

          if (!ok) continue;

          // TODO: _ruleE++;
          continueReductions = true;

          if (placeInQuery[p] == 0)
              reducer->parent->initialMarking[p] = 0;

          bool skipplace = (notenabled.size() == place.consumers.size()) && (placeInQuery[p] == 0);
          for (uint cons : notenabled)
              skipTransition(cons);

          if (skipplace)
              skipPlace(p);

      }
      assert(reducer->consistent());
      return continueReductions;
  }
}