//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEB_H
#define VERIFYPN_REDUCTIONRULEB_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleB : public ReductionRule {
    public:
      explicit ReductionRuleB(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
  };
}

#endif //VERIFYPN_REDUCTIONRULEB_H
