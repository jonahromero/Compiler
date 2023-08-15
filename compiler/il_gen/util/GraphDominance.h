#pragma once
#include <unordered_map>
#include <set>
#include "CFG.h"

std::unordered_map<size_t, std::vector<size_t>> dominanceFrontier(CFG const& graph);