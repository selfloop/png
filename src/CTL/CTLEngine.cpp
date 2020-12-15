#include "CTL/CTLEngine.h"

#include "CTL/PetriNets/OnTheFlyDG.h"
#include "CTL/CTLResult.h"


#include "CTL/Algorithm/CertainZeroFPA.h"
#include "CTL/Algorithm/LocalFPA.h"


#include "CTL/Stopwatch.h"
#include "PetriEngine/options.h"
#include "PetriEngine/Reachability/ReachabilityResult.h"
#include "PetriEngine/TAR/TARReachability.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <PetriEngine/PQL/Expressions.h>

using namespace CTL;
using namespace PetriEngine;
using namespace PetriEngine::PQL;
using namespace PetriEngine::Reachability;
using namespace PetriNets;

ReturnValue getAlgorithm(std::shared_ptr<Algorithm::FixedPointAlgorithm>& algorithm,
                         CTLAlgorithmType algorithmtype, Reachability::Strategy search)
{
    switch(algorithmtype)
    {
        case CTLAlgorithmType::Local:
            algorithm = std::make_shared<Algorithm::LocalFPA>(search);
            break;
        case CTLAlgorithmType::CZero:
            algorithm = std::make_shared<Algorithm::CertainZeroFPA>(search);
            break;
        default:
            cerr << "Error: Unknown or unsupported algorithm" << endl;
            return ErrorCode;
    }
    return ContinueCode;
}

void printResult(const std::string& qname, CTLResult& result, bool statisticslevel, bool mccouput, bool only_stats, size_t index, options_t& options){
    const static string techniques = "TECHNIQUES COLLATERAL_PROCESSING EXPLICIT STATE_COMPRESSION SAT_SMT ";

    if(!only_stats)
    {
        cout << endl;
        cout << "FORMULA "
             << qname
             << " " << (result.result ? "TRUE" : "FALSE") << " "
             << techniques
             << (options.isCPN ? "UNFOLDING_TO_PT " : "")
             << (options.stubbornreduction ? "STUBBORN_SETS " : "")
             << (options.ctlalgorithm == CTL::CZero ? "CTL_CZERO " : "")
             << (options.ctlalgorithm == CTL::Local ? "CTL_LOCAL " : "")
                << endl << endl;
        std::cout << "Query index " << index << " was solved" << std::endl;
        cout << "Query is" << (result.result ? "" : " NOT") << " satisfied." << endl;

        cout << endl;
    }
    if(statisticslevel){
        cout << "STATS:" << endl;
        cout << "	Time (seconds)    : " << setprecision(4) << result.duration / 1000 << endl;
        cout << "	Configurations    : " << result.numberOfConfigurations << endl;
        cout << "	Markings          : " << result.numberOfMarkings << endl;
        cout << "	Edges             : " << result.numberOfEdges << endl;
        cout << "	Processed Edges   : " << result.processedEdges << endl;
        cout << "	Processed N. Edges: " << result.processedNegationEdges << endl;
        cout << "	Explored Configs  : " << result.exploredConfigurations << endl;
        std::cout << endl;
    }
}

bool singleSolve(const Condition_ptr& queryPointer, PetriNet* net, CTLAlgorithmType algorithmType,
                 Strategy strategyType, bool partialOrder, CTLResult& result)
{
    OnTheFlyDG graph(net, partialOrder);
    graph.setQuery(queryPointer);
    std::shared_ptr<Algorithm::FixedPointAlgorithm> alg = nullptr;
    if(getAlgorithm(alg, algorithmType, strategyType) == ErrorCode){
        assert(false);
    }

    stopwatch timer;
    timer.start();
    auto res = alg->search(graph);
    timer.stop();

    result.duration += timer.duration();
    result.numberOfConfigurations += graph.configurationCount();
    result.numberOfMarkings += graph.markingCount();
    result.processedEdges += alg->processedEdges();
    result.processedNegationEdges += alg->processedNegationEdges();
    result.exploredConfigurations += alg->exploredConfigurations();
    result.numberOfEdges += alg->numberOfEdges();
    return res;
}

bool recursiveSolve(const Condition_ptr& queryPointer, PetriNet* net,
                    CTLAlgorithmType algorithmType,
                    Strategy strategyType, bool partialOrder, CTLResult& result, options_t& options);

bool solveTrivially(const PetriNet *net, bool partial_order, CTLResult &resultObject, bool &solved);

class ResultHandler : public AbstractHandler {
    private:
        bool _is_conj = false;
        const std::vector<int8_t>& _lstate;
    public:
        ResultHandler(bool is_conj, const std::vector<int8_t>& lstate)
        : _is_conj(is_conj), _lstate(lstate)
        {}
        
        std::pair<AbstractHandler::Result, bool> handle(
                size_t index,
                PQL::Condition* query, 
                AbstractHandler::Result result,
                const std::vector<uint32_t>* maxPlaceBound,
                size_t expandedStates,
                size_t exploredStates,
                size_t discoveredStates,
                int maxTokens,                
                Structures::StateSetInterface* stateset, size_t lastmarking, const MarkingValue* initialMarking) override
        {
            if(result == ResultPrinter::Satisfied)
            {
                result = _lstate[index] < 0 ? ResultPrinter::NotSatisfied : ResultPrinter::Satisfied;
            }
            else if(result == ResultPrinter::NotSatisfied)
            {
                result = _lstate[index] < 0 ? ResultPrinter::Satisfied : ResultPrinter::NotSatisfied;
            }
            bool terminate = _is_conj ? (result == ResultPrinter::NotSatisfied) : (result == ResultPrinter::Satisfied);
            return std::make_pair(result, terminate);            
        }
};

bool solveLogicalCondition(LogicalCondition* query, bool is_conj, PetriNet* net,
                           CTLAlgorithmType algorithmtype,
                           Reachability::Strategy strategytype, bool partial_order, CTLResult& result, options_t& options)
{
    std::vector<int8_t> state(query->size(), 0);
    std::vector<int8_t> lstate;
    std::vector<Condition_ptr> queries;
    for(size_t i = 0; i < query->size(); ++i)
    {
        if((*query)[i]->isReachability())
        {
            state[i] = dynamic_cast<NotCondition*>((*query)[i].get()) ? -1 : 1;
            queries.emplace_back((*query)[i]->prepareForReachability());
            lstate.emplace_back(state[i]);
        }
    }

    {
        ResultHandler handler(is_conj, lstate);
        std::vector<AbstractHandler::Result> res(queries.size(), AbstractHandler::Unknown);
        if(!options.tar)
        {
            ReachabilitySearch strategy(*net, handler, options.kbound, true);
            strategy.reachable(queries, res,
                                        options.strategy,
                                        options.stubbornreduction,
                                        false,
                                        false,
                                        false);
        }
        else
        {
            TARReachabilitySearch tar(handler, *net, nullptr, options.kbound);
            tar.reachable(queries, res, false, false);
        }
        size_t j = 0;
        for(size_t i = 0; i < query->size(); ++i) {
            if (state[i] != 0)
            {
                if (res[j] == AbstractHandler::Unknown) {
                    ++j;
                    continue;
                }
                auto bres = res[j] == AbstractHandler::Satisfied;

                if(bres xor is_conj) {
                    return !is_conj;
                }
                ++j;
            }
        }
    }

    for(size_t i = 0; i < query->size(); ++i) {
        if (state[i] == 0)
        {
            if(recursiveSolve((*query)[i], net, algorithmtype, strategytype, partial_order, result, options) xor is_conj)
            {
                return !is_conj;
            }
        }
    }
    return is_conj;
}

class SimpleResultHandler : public AbstractHandler
{
public:
    std::pair<AbstractHandler::Result, bool> handle(
                size_t index,
                PQL::Condition* query, 
                AbstractHandler::Result result,
                const std::vector<uint32_t>* maxPlaceBound,
                size_t expandedStates,
                size_t exploredStates,
                size_t discoveredStates,
                int maxTokens,                
                Structures::StateSetInterface* stateset, size_t lastmarking, const MarkingValue* initialMarking) {
        return std::make_pair(result, false);
    }
};

bool recursiveSolve(const Condition_ptr& queryPointer, PetriEngine::PetriNet* net, CTL::CTLAlgorithmType algorithmType,
                    PetriEngine::Reachability::Strategy strategyType, bool partialOrder, CTLResult& result,
                    options_t& options)
{
    if(auto q = dynamic_cast<NotCondition*>(queryPointer.get()))
    {
        return ! recursiveSolve((*q)[0], net, algorithmType, strategyType, partialOrder, result, options);
    }
    else if(auto q = dynamic_cast<AndCondition*>(queryPointer.get()))
    {
        return solveLogicalCondition(q, true, net, algorithmType, strategyType, partialOrder, result, options);
    }
    else if(auto q = dynamic_cast<OrCondition*>(queryPointer.get()))
    {
        return solveLogicalCondition(q, false, net, algorithmType, strategyType, partialOrder, result, options);
    }
    else if(queryPointer->isReachability())
    {
        SimpleResultHandler handler;
        std::vector<Condition_ptr> queries{queryPointer->prepareForReachability()};
        std::vector<AbstractHandler::Result> res;
        res.emplace_back(AbstractHandler::Unknown);
        if(options.tar)
        {
            TARReachabilitySearch tar(handler, *net, nullptr, options.kbound);
            tar.reachable(queries, res, false, false);
        }
        else
        {
            ReachabilitySearch strategy(*net, handler, options.kbound, true);
            strategy.reachable(queries, res,
                           options.strategy,
                           options.stubbornreduction,
                           false,
                           false,
                           false);
        }
        return (res.back() == AbstractHandler::Satisfied) xor queryPointer->isInvariant();
    }
    else
    {
        return singleSolve(queryPointer, net, algorithmType, strategyType, partialOrder, result);
    }
}

bool solveTrivially(PetriNet *net, bool partial_order, CTLResult &resultObject, bool &solved) {
    OnTheFlyDG graph(net, partial_order);
    graph.setQuery(resultObject.query);
    switch (graph.initialEval()) {
        case Condition::RFALSE:
            resultObject.result = false;
            return true;
        case Condition::RTRUE:
            resultObject.result = true;
            return true;
        default:
            return false;
    }
}


void setFieldsToZero(CTLResult &resultObject) {
    resultObject.numberOfConfigurations = 0;
    resultObject.numberOfMarkings = 0;
    resultObject.processedEdges = 0;
    resultObject.processedNegationEdges = 0;
    resultObject.exploredConfigurations = 0;
    resultObject.numberOfEdges = 0;
    resultObject.duration = 0;
}

ReturnValue CTLMain(
        PetriNet* net,
        CTLAlgorithmType algorithmType,
        Strategy strategyType,
        bool gamemode,
        bool printStatistics,
        bool mccOutput,
        bool isPartialOrder,
        const std::vector<std::string>& querynames,
        const std::vector<std::shared_ptr<Condition>>& queries,
        const std::vector<size_t>& querynumbers,
        options_t& options
        ){
    for(auto queryNumber : querynumbers){
        CTLResult resultObject(queries[queryNumber]);
        bool solved = false;

        solveTrivially(net, isPartialOrder, resultObject, solved);

        setFieldsToZero(resultObject);

        if(!solved){
            resultObject.result = recursiveSolve(resultObject.query, net, algorithmType, strategyType, isPartialOrder,
                                                 resultObject, options);
        }
        printResult(querynames[queryNumber], resultObject, printStatistics, mccOutput, false, queryNumber,options);
    }
    return SuccessCode;
}

