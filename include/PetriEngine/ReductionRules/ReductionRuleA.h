//
// Created by hamburger on 04.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEA_H
#define VERIFYPN_REDUCTIONRULEA_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleA : public ReductionRule {
    public:
      explicit ReductionRuleA(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
  };
}

#endif //VERIFYPN_REDUCTIONRULEA_H
