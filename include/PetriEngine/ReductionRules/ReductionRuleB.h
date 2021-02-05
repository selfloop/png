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
      ReductionRuleB(
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

      bool reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) override;
    private:
      bool reconstructTrace;
      std::vector<std::string> &_initfire;
      std::unordered_map<std::string, std::vector<std::string>> &_postfire;
      std::unordered_map<std::string, std::vector<ExpandedArc>> &_extraconsume;
  };
}

#endif //VERIFYPN_REDUCTIONRULEB_H
