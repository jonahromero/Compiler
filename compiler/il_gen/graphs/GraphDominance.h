#pragma once
#include "IL.h"
#include <unordered_map>
#include <set>

using NodeSet = std::set<IL::NodePtr>;

auto calculateDominanceFrontiers(IL::NodePtr const& entryNode)->std::unordered_map<IL::NodePtr, NodeSet>;
auto calculateDominances(IL::NodePtr const& entryNode)->std::unordered_map<IL::NodePtr, NodeSet>;
