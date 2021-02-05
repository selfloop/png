//
// Created by hamburger on 04.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULE_H
#define VERIFYPN_REDUCTIONRULE_H

#include "../PetriNet.h"
#include "../PQL/Contexts.h"
#include "../../PetriParse/PNMLParser.h"
#include "../NetStructures.h"
#include <PetriEngine/PetriNetBuilder.h>

#include <vector>

namespace PetriEngine {

  class ReductionRule {
    public:
      explicit ReductionRule(Reducer *reducer): reducer(reducer)  {};

      virtual bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) = 0;
    protected:
      Reducer *reducer;
      std::string getTransitionName(uint32_t transition);
      Transition &getTransition(uint32_t transition);
      static void eraseTransition(std::vector<uint32_t> &, uint32_t);
      void skipTransition(uint32_t);
      void skipPlace(uint32_t);
  };
}

#endif //VERIFYPN_REDUCTIONRULE_H
