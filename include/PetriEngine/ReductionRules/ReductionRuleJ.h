//
// Created by hamburger on 05.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEJ_H
#define VERIFYPN_REDUCTIONRULEJ_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleJ : public ReductionRule {
    public:
      explicit ReductionRuleJ(Reducer *reducer) : ReductionRule(reducer) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      size_t _tnameid = 0;

      std::string newTransName();
  };
}

#endif //VERIFYPN_REDUCTIONRULEJ_H
