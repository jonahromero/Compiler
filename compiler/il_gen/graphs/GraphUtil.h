#pragma once
#include "IL.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>

template<typename Callable>
void depthFirstVisit(IL::NodePtr const& entryNode, Callable callback) {
	std::unordered_set<IL::NodePtr> nodesVisited;
	std::queue<IL::NodePtr> nodesLeft; nodesLeft.push(entryNode);
	while (!nodesLeft.empty()) {
		auto& node = nodesLeft.front();
		if (!nodesVisited.count(node) && node) {
			callback(node);
			for (auto& child : node->children) {
				nodesLeft.push(child);
			}
		}
		nodesVisited.insert(node);
		nodesLeft.pop();
	}
}