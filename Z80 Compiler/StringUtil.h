#pragma once
#include <string>
#include <math.h>
#include "CharacterUtil.h"

inline bool decimalStrGreaterThanOrEqual(std::string_view first, std::string_view second) {
	if (first.size() == second.size()) { return first >= second; }
	return first.size() > second.size();
}

class IntegralTypeTooSmall
	: public std::exception {};

template<std::integral T>
T hexToIntegral(std::string_view hex) {
	if (sizeof(T) * 2 < hex.size()) throw IntegralTypeTooSmall();
	T retval = 0, exponent = 1;
	for (auto it = hex.rbegin(); it != hex.rend(); it++) {
		retval += exponent * hexDigitToUInt8(*it);
		exponent *= 16;
	}
	return retval;
}

template<std::integral T>
T decToIntegral(std::string_view decimal) {
	if (decimalStrGreaterThanOrEqual(decimal, std::to_string(std::numeric_limits<T>::max()))) {
		throw IntegralTypeTooSmall();
	}
	T retval = 0, exponent = 1;
	for (auto it = decimal.rbegin(); it != decimal.rend(); it++) {
		retval += exponent * decDigitToUInt8(*it);
		exponent *= 10;
	}
	return retval;

}

template<std::integral T>
T binaryToIntegral(std::string_view binary) {
	if(sizeof(T) * 8 < binary.size()) throw IntegralTypeTooSmall();
	T retval = 0;
	for (auto l : binary) {
		retval <<= 1;
		retval |= l - '0';
	}
	return retval;
}

inline auto toLower(std::string_view str) -> std::string {
	std::string retval; retval.reserve(str.size());
	for (char l : str) {
		retval.push_back(isUppercase(l) ? l - 'A' + 'a' : l);
	}
	return retval;
}

namespace util {
	namespace detail {
		constexpr size_t textSize(char a) { return 1; }
		constexpr size_t textSize(std::string_view a) { return a.size(); }
		template<typename T>
		void addToStr(std::string& str, T&& value) {
			using U = std::remove_cvref_t<T>;
			if constexpr (std::is_same_v<U, char>) {
				str.push_back(value);
			}
			else {
				str.append(std::forward<T>(value));
			}
		}
	}
	template<typename...Args>
	auto strBuilder(Args&&...args) -> std::string {
		std::string str; str.reserve((textSize(args) + ...));
		(addToStr(str, std::forward<Args>(args)), ...);
		return str;
	}
}