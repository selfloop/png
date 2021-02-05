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
      explicit ReductionRuleG(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
  };
}

#endif //VERIFYPN_REDUCTIONRULEG_H
