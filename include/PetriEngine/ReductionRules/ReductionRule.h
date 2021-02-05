//
// Created by hamburger on 04.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULE_H
#define VERIFYPN_REDUCTIONRULE_H

#include "../PetriNet.h"
#include "../PQL/Contexts.h"
#include "../../PetriParse/PNMLParser.h"
#include "../NetStructures.h"
#include <PetriEngine/PetriNetBuilder.h>

#include <vector>

namespace PetriEngine {

  class ReductionRule {
    public:
      ReductionRule(
          PetriNetBuilder *parent,
          std::chrono::high_resolution_clock::time_point *timer,
          int timeout,
          std::vector<uint32_t> &skipped_trans,
          size_t *_removedTransitions,
          size_t *_removedPlaces
      ) : parent(parent),
          timer(timer),
          timeout(timeout),
          _skipped_trans(skipped_trans),
          _removedTransitions(_removedTransitions),
          _removedPlaces(_removedPlaces) {};
      virtual bool reduce(uint32_t *placeInQuery) = 0;
    protected:
      size_t *_removedTransitions = nullptr;
      size_t *_removedPlaces = nullptr;
      PetriNetBuilder *parent = nullptr;
      int timeout = 0;
      std::chrono::high_resolution_clock::time_point *timer;
      std::vector<uint32_t> &_skipped_trans;

      std::string getTransitionName(uint32_t transition);
      std::string getPlaceName(uint32_t place);
      Transition &getTransition(uint32_t transition);
      static ArcIter getInArc(uint32_t place, Transition &);
      static ArcIter getOutArc(Transition &, uint32_t place);
      static void eraseTransition(std::vector<uint32_t> &, uint32_t);
      void skipTransition(uint32_t);
      void skipPlace(uint32_t);
      bool consistent();
      [[nodiscard]] bool hasTimedout() const {
          auto end = std::chrono::high_resolution_clock::now();
          auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - *timer);
          return (diff.count() >= timeout);
      }
  };
}

#endif //VERIFYPN_REDUCTIONRULE_H
