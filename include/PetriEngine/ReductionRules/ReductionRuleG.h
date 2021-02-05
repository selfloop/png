//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEG_H
#define VERIFYPN_REDUCTIONRULEG_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleG : public ReductionRule {
    public:
      ReductionRuleG(
          PetriNetBuilder *parent,
          std::chrono::high_resolution_clock::time_point *timer,
          int timeout,
          bool reconstructTrace,
          std::vector<uint32_t> &skipped_trans,
          size_t *_removedTransitions,
          size_t *_removedPlaces
      ) : ReductionRule(parent, timer, timeout, skipped_trans, _removedTransitions, _removedPlaces),
          reconstructTrace(reconstructTrace) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      bool reconstructTrace;
  };
}

#endif //VERIFYPN_REDUCTIONRULEG_H
