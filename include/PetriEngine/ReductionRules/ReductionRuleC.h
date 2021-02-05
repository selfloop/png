//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEC_H
#define VERIFYPN_REDUCTIONRULEC_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleC : public ReductionRule {
    public:
      explicit ReductionRuleC(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      std::vector<uint8_t> _pflags;
  };
}

#endif //VERIFYPN_REDUCTIONRULEC_H
