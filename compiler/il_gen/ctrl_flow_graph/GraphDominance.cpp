#include "CtrlFlowGraph.h"
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using DominatorSet = std::vector<std::pair<size_t, std::set<size_t>>>;

DominatorSet calculateDominatorSets(CtrlFlowGraph const& graph)
{
    DominatorSet dominatorSets;
    for (size_t avoid_node = 0; avoid_node < graph.nodeCount(); avoid_node++) 
    {
        std::stack<size_t> worklist;
        worklist.push(graph.getEntryNode());
        std::set<size_t> visited;
        while (!worklist.empty()) 
        {
            auto current = worklist.top(); worklist.pop();
            if (current == avoid_node) continue;
            visited.insert(current);
            for (auto succ : graph.out(current)) {
                worklist.push(succ);
            }
        }
        dominatorSets.emplace_back(avoid_node, std::move(visited));
    }
    return dominatorSets;
}

PureGraph calculateIdom(CtrlFlowGraph const& graph)
{
    auto idom = PureGraph::trivialGraph(graph.nodeCount());
    auto dominatorSets = calculateDominatorSets(graph);
    std::sort(dominatorSets.begin(), dominatorSets.end(), [](auto& lhs, auto& rhs) 
    {
        return lhs.second.size() < rhs.second.size();
    });

    std::unordered_set<size_t> worklist = {};
    for (auto& [node, dominees] : dominatorSets) 
    {
        if (dominees.empty()) { // this node is a leaf node
            worklist.insert(node);
            continue;
        }
        for (auto& succ : dominees) 
        {
            if (worklist.count(succ) != 0) {
                worklist.erase(succ);
                idom.addEdge(node, succ);
            }
        }
    }
    return idom;
}

std::vector<size_t> dominanceFrontierForNode(
    CtrlFlowGraph const& graph, 
    PureGraph const& idom, 
    size_t node,
    std::unordered_map<size_t, std::vector<size_t>> frontiers
) 
{
    std::vector<size_t> frontier;
    for (auto succ : graph.out(node)) 
    {
        if (!idom.hasEdge(node, succ)) {
            frontier.push_back(succ);
        }
    }
    for (auto child : idom.out(node)) {
        for (auto distant_succ : frontiers[child]) {
            if (!idom.hasEdge(node, distant_succ)) {
                frontier.push_back(distant_succ);
            }
        }
    }
    return frontier;
}

std::unordered_map<size_t, std::vector<size_t>> dominanceFrontier(CtrlFlowGraph const& graph)
{
    std::unordered_map<size_t, std::vector<size_t>> frontiers;
    PureGraph idom = calculateIdom(graph);
    idom.dfs(graph.getEntryNode(), [&](size_t visiting) {
        frontiers.emplace(visiting, dominanceFrontierForNode(graph, idom, visiting, frontiers));
    });
    return frontiers;
}
