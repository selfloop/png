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
      ReductionRuleC(
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
      std::vector<uint8_t> _pflags;
  };
}

#endif //VERIFYPN_REDUCTIONRULEC_H
