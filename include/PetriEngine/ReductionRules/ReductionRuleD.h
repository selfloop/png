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
      explicit ReductionRuleD(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      std::vector<uint8_t> _tflags;
  };
}

#endif //VERIFYPN_REDUCTIONRULED_H
