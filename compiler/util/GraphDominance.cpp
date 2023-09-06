#include "Graph.h"
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <iostream>

using DominatorSet = std::vector<std::pair<size_t, std::set<size_t>>>;

DominatorSet calculateDominatorSets(size_t entry, PureGraph const& graph)
{
    DominatorSet dominatorSets;
    for (size_t avoid_node = 0; avoid_node < graph.nodeCount(); avoid_node++) 
    {
        std::stack<size_t> worklist;
        worklist.push(entry);
        std::set<size_t> unreachable, visited;
        for (size_t i = 0; i < graph.nodeCount(); ++i) {
            if(i != avoid_node) unreachable.insert(i);
        }
        while (!worklist.empty()) 
        {
            auto current = worklist.top(); worklist.pop();
            if (current == avoid_node || visited.count(current) != 0) continue;
            visited.insert(current);
            unreachable.erase(current);
            for (auto succ : graph.out(current)) {
                worklist.push(succ);
            }
        }
        dominatorSets.emplace_back(avoid_node, std::move(unreachable));
    }
    return dominatorSets;
}

PureGraph calculateIdom(size_t entry, PureGraph const& graph)
{
    auto idom = PureGraph::trivialGraph(graph.nodeCount());
    auto dominatorSets = calculateDominatorSets(entry, graph);
    std::sort(dominatorSets.begin(), dominatorSets.end(), [](auto& lhs, auto& rhs) 
    {
        return lhs.second.size() < rhs.second.size();
    });

    std::unordered_set<size_t> leaves = {};
    for (auto& [node, dominees] : dominatorSets) 
    {
        for (auto& dominee : dominees)
        {
            if (leaves.count(dominee) != 0) {
                leaves.erase(dominee);
                idom.addEdge(node, dominee);
            }
        }
        leaves.insert(node);
    }
    return idom;
}

std::vector<size_t> dominanceFrontierForNode(
    PureGraph const& graph,
    PureGraph const& idom, 
    size_t node,
    std::unordered_map<size_t, std::vector<size_t>> const& frontiers
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
        if (frontiers.find(child) == frontiers.end()) continue;
        for (auto distant_succ : frontiers.at(child)) {
            if (!idom.hasEdge(node, distant_succ)) {
                frontier.push_back(distant_succ);
            }
        }
    }
    return frontier;
}

std::unordered_map<size_t, std::vector<size_t>> dominanceFrontier(size_t entry, size_t exit, PureGraph const& graph)
{
    PureGraph modifiedGraph = graph;
    modifiedGraph.addEdge(entry, exit);
    std::unordered_map<size_t, std::vector<size_t>> frontiers;
    PureGraph idom = calculateIdom(entry, modifiedGraph);
    std::cout << "IDOM: " << std::endl;
    std::cout << idom;
    std::cout << "VISITING:\n";
    idom.dfs(entry, [&](size_t visiting) {
        std::cout << visiting << '\n';
        frontiers.emplace(visiting, dominanceFrontierForNode(modifiedGraph, idom, visiting, frontiers));
    });
    return frontiers;
}
