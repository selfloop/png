#ifndef CTLENGINE_H
#define CTLENGINE_H

#include "../PetriEngine/errorcodes.h"
#include "../PetriEngine/PetriNet.h"
#include "../PetriEngine/Reachability/ReachabilitySearch.h"

#include "Algorithm/AlgorithmTypes.h"
#include "../PetriEngine/PQL/PQL.h"

#include <set>

ReturnValue CTLMain(PetriEngine::PetriNet* net,
                    CTL::CTLAlgorithmType algorithmType,
                    PetriEngine::Reachability::Strategy strategyType,
                    bool gamemode,
                    bool printStatistics,
                    bool mccOutput,
                    bool isPartialOrder,
                    const std::vector<std::string>& querynames,
                    const std::vector<std::shared_ptr<PetriEngine::PQL::Condition>>& reducedQueries,
                    const std::vector<size_t>& ids,
                    options_t& options);

#endif // CTLENGINE_H
