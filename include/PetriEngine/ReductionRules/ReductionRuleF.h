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
      ReductionRuleF(
          PetriNetBuilder *parent,
          std::chrono::high_resolution_clock::time_point *timer,
          int timeout,
          bool reconstructTrace,
          std::unordered_map<std::string, std::vector<ExpandedArc>> &extraconsume,
          std::vector<uint32_t> &skipped_trans,
          size_t *_removedTransitions,
          size_t *_removedPlaces
      ) : ReductionRule(parent, timer, timeout, skipped_trans, _removedTransitions, _removedPlaces),
          reconstructTrace(reconstructTrace),
          _extraconsume(extraconsume) {};

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      bool reconstructTrace;
      std::unordered_map<std::string, std::vector<ExpandedArc>> &_extraconsume;
  };
}

#endif //VERIFYPN_REDUCTIONRULEF_H
