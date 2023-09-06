#pragma once
#include <string>
#include <math.h>
#include "CharacterUtil.h"

	
namespace util
{
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

	namespace detail
	{	
		inline std::string_view removeLeadingZeroes(std::string_view view) {
			auto i = std::find_if(view.begin(), view.end(), [](auto& c) { return c != '0'; });
			return view.substr(std::distance(view.begin(), i));
		}
		inline bool decimalStrGreaterThanOrEqual(std::string_view first, std::string_view second) {
			first = removeLeadingZeroes(first); second = removeLeadingZeroes(second);
			if (first.size() == second.size()) { return first >= second; }
			return first.size() > second.size();
		}
	}

	template<std::integral T>
	T decToIntegral(std::string_view decimal) {
		if (detail::decimalStrGreaterThanOrEqual(decimal, std::to_string(std::numeric_limits<T>::max()))) {
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

	inline auto toStringWithOrdinalSuffix(size_t i) 
	{
		std::string asStr = std::to_string(i);
		switch (i % 10) {
		case 1:	return asStr + "st";
		case 2:	return asStr + "nd";
		case 3:	return asStr + "rd";
		default: return asStr + "th";
		}
	}

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

	inline std::string joinVector(std::string_view inbetween, std::vector<std::string> const& items) {
		std::string retval; 
		for (int i = 0; i < items.size(); i++) {
			retval.append(items[i]);
			if (i != items.size() - 1) {
				retval += inbetween;
			}
		}
		return retval;
	}

	template<typename...Args>
	auto strBuilder(Args&&...args) -> std::string {
		std::string str; str.reserve((detail::textSize(args) + ...));
		(detail::addToStr(str, std::forward<Args>(args)), ...);
		return str;
	}

	struct StringHash
	{
		using hash_type = std::hash<std::string_view>;
		using is_transparent = void;

		std::size_t operator()(const char* str) const { return hash_type{}(str); }
		std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
		std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
	};
}