#include "GraphDominance.h"
#include "GraphUtil.h"
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <queue>

using DominatorSet = std::vector<std::set<size_t>>;

DominatorSet initDominatorSets(size_t states, size_t entryState) {
    std::set<size_t> fullSet;
    for (size_t i = 0; i < states; i++) {
        fullSet.insert(i);
    }
    DominatorSet dominatorSets(graph.nodeCount());
    for(size_t i = 0; i < graph.nodeCount(); i++){
        if(i != entryState) dominatorSets[i] = fullSet;
    }
    dominatorSets[entryState] = std::set<size_t>{{entryState}};
    return dominatorSets;
}

std::set<size_t> getIntersectionOfPred(size_t node, DominatorSet const& dominators) 
{
    std::set<size_t> intersection;
    bool firstIter = true;
    for(auto graphPred : graph.pred(node)) {
        if(firstIter) { intersection = dominators[graphPred]; continue; }
        std::set<size_t> temp;
        std::set<size_t>& predSet = dominators[graphPred];
        std::set_intersection(predSet.begin(), predSet.end(), intersection.begin(), intersection.end(), std::inserter(temp, temp.begin()));
        intersection = temp;
    }
    return intersection;
}

DominatorSet calculateDominatorSets(CFG const& graph) 
{
    auto dominatorSets = initDominatorSets(graph.getEntryNode(), graph.getEntryNode());
    std::stack<size_t> worklist; 
    for(auto out : graph.out(graph.getEntryNode())) {
        worklist.push(out);
    }
    while(!worklist.empty()) 
    {
        auto node = worklist.top(); worklist.pop();
        auto& currentDominatorSet = dominatorSets[node];
        auto intersection = getIntersectionOfPred(node, dominatorSets);
        bool newNodesAdded = false;
        for(auto& n : intersection) {
            if(!currentDominatorSet.insert(n).second) 
                newNodesAdded = true;
        }
        if(newNodesAdded) {
            for(auto succ : graph.out(node)) {
                worklist.push(succ);
            }
        }
    }
    return dominatorSets;
}

PureGraph calculateIdom(CFG const& graph)
{
    auto dominatorSets = calculateDominatorSets(graph);
    std::sort(dominatorSets.begin(), dominatorSets.end(), [](auto& lhs, auto& rhs) {
        return lhs.
    });
}

/*
// algorithms
PureGraph createDominator(CFG const& graph) {
    auto idom = PureGraph::trivialGraph(graph.nodeCount());
    auto dominatorSets = calculateDominatorSets(graph);
    std::set<size_t> nodesAdded;

    for(size_t i = 0; i < graph.nodeCount(); i++) {
        idom
    }
}
*/