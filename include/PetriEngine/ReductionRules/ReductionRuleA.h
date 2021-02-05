//
// Created by hamburger on 04.02.21.
//

#ifndef VERIFYPN_REDUCTIONRULEA_H
#define VERIFYPN_REDUCTIONRULEA_H

#include <PetriEngine/PetriNetBuilder.h>
#include "ReductionRule.h"

namespace PetriEngine {
  class ReductionRuleA : public ReductionRule {
    public:
      ReductionRuleA(
          PetriNetBuilder *parent,
          std::chrono::high_resolution_clock::time_point *timer,
          int timeout,
          bool reconstructTrace,
          std::vector<std::string> &initfire,
          std::unordered_map<std::string, std::vector<std::string>> &postfire,
          std::unordered_map<std::string, std::vector<ExpandedArc>> &extraconsume,
          std::vector<uint32_t> &skipped_trans,
          size_t *_removedTransitions,
          size_t *_removedPlaces
      ) : ReductionRule(parent, timer, timeout, skipped_trans, _removedTransitions, _removedPlaces),
          reconstructTrace(reconstructTrace),
          _initfire(initfire),
          _postfire(postfire),
          _extraconsume(extraconsume) {};

      bool reduce(uint32_t *placeInQuery) override;
    private:
      bool reconstructTrace;
      std::vector<std::string> &_initfire;
      std::unordered_map<std::string, std::vector<std::string>> &_postfire;
      std::unordered_map<std::string, std::vector<ExpandedArc>> &_extraconsume;
  };
}

#endif //VERIFYPN_REDUCTIONRULEA_H
