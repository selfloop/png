//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEF_H
#define VERIFYPN_REDUCTIONRULEF_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleF : public ReductionRule {
    public:
      explicit ReductionRuleF(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
  };
}

#endif //VERIFYPN_REDUCTIONRULEF_H
