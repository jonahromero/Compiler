#include "GraphDominance.h"
#include "GraphUtil.h"
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <queue>

template<typename T>
auto setDifference(std::set<T> const& first, std::set<T> const& other) {
	std::set<T> intersection;
	std::set_difference(first.begin(), first.end(), other.begin(), other.end(), std::inserter(intersection, intersection.begin()));
	return intersection;
}

auto getAllNodes(IL::NodePtr const& entryNode)->std::set<IL::NodePtr> {
	std::set<IL::NodePtr> nodes;
	depthFirstVisit(entryNode, [&](IL::NodePtr const& node) {
		nodes.insert(node);
	});
	return nodes;
}

auto getNodesAvoidingNode(IL::NodePtr const& entryNode, IL::NodePtr const& toAvoid) -> std::set<IL::NodePtr> {
	std::set<IL::NodePtr> visitedNodes;
	std::queue<IL::NodePtr> nodesToVisit;
	nodesToVisit.push(entryNode);
	while(!nodesToVisit.empty()) {
		auto& topNode = nodesToVisit.front();
		if (topNode.get() != toAvoid.get() && !visitedNodes.contains(topNode)) {
			visitedNodes.insert(topNode);
			for (auto& child : topNode->children) {
				nodesToVisit.push(child);
			}
		}
		nodesToVisit.pop();
	}
	return visitedNodes;
}

auto calculateDominanceFrontiers(IL::NodePtr const& entryNode) -> std::unordered_map<IL::NodePtr, NodeSet>
{
	// for each node x, look at all nodes z, and see if x doesnt dominate z, but dominates one of its predecessors
	// place phi nodes at the dominance frontiers
	auto dominances = calculateDominances(entryNode);
	std::unordered_map<IL::NodePtr, NodeSet> dominanceFrontiers;
	depthFirstVisit(entryNode, [&](IL::NodePtr const& nodeX) {
		depthFirstVisit(nodeX, [&](IL::NodePtr const& nodeZ) {
			//dominates a predeccessor
			if(dominances[nodeX].contains(nodeZ)) {
				// doesnt dominate child
				for (auto& child : nodeZ->children) {
					if (!dominances[nodeX].contains(child)) {
						dominanceFrontiers[nodeX].insert(child);
					}
				}
			}
		});
	});
	return dominanceFrontiers;
}

auto calculateDominances(IL::NodePtr const& entryNode) -> std::unordered_map<IL::NodePtr, NodeSet>
{
	auto allNodes = getAllNodes(entryNode);
	std::unordered_map<IL::NodePtr, NodeSet> dominances;
	depthFirstVisit(entryNode, [&](IL::NodePtr& node) {
		auto nodes_avoided = getNodesAvoidingNode(entryNode, node);
		auto intersection = setDifference(allNodes, nodes_avoided);
		dominances.emplace(node, std::move(intersection));
	});
	return dominances;
}
