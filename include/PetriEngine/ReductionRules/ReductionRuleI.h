//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEI_H
#define VERIFYPN_REDUCTIONRULEI_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleI : public ReductionRule {
    public:
      explicit ReductionRuleI(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
  };
}

#endif //VERIFYPN_REDUCTIONRULEI_H
