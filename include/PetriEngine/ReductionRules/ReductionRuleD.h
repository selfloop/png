//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULED_H
#define VERIFYPN_REDUCTIONRULED_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleD : public ReductionRule {
    public:
      ReductionRuleD(
          PetriNetBuilder *parent,
          std::chrono::high_resolution_clock::time_point *timer,
          int timeout,
          std::vector<uint32_t> &skipped_trans,
          size_t *_removedTransitions,
          size_t *_removedPlaces
      ) : ReductionRule(parent, timer, timeout, skipped_trans, _removedTransitions, _removedPlaces) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      std::vector<uint8_t> _tflags;
  };
}

#endif //VERIFYPN_REDUCTIONRULEC_H
