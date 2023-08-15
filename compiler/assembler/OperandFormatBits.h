#pragma once
#include <unordered_map>
#include <optional>
#include "StringUtil.h"

namespace Asm {
	using Bits = uint8_t;
	
	std::optional<Bits> reg8ToBits(std::string_view reg) {
		static std::unordered_map<std::string_view, uint8_t> map = {
			{"b", 0}, {"c", 1}, {"d", 2}, {"e", 3},{"a", 7},
			{"h", 4}, {"l", 5}, {"ixh", 4}, {"ixl", 5},{"iyh", 4}, {"iyl", 5}
		};
		auto it = map.find(toLower(reg));
		return it == map.end() ? std::nullopt : std::optional(it->second);
	}

	std::optional<Bits> reg16ToBits(std::string_view reg) {
		static std::unordered_map<std::string_view, uint8_t> map = {
			{"bc", 0}, {"de", 1}, {"hl", 2}, {"sp", 3}
		};
		auto it = map.find(toLower(reg));
		return it == map.end() ? std::nullopt : std::optional(it->second);
	}

	std::optional<Bits> flagToBits(std::string_view flag) {
		static std::unordered_map<std::string_view, uint8_t> map = {
			{"nz", 0}, {"z", 1}, {"nc", 2}, {"c", 3}, {"po", 4}, 
			{"pe", 5}, {"p", 6}, {"m", 7}
		};
		auto it = map.find(toLower(flag));
		return it == map.end() ? std::nullopt : std::optional(it->second);
	}
}