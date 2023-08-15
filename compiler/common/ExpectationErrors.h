#pragma once
#include <stdexcept>
#include <optional>

template<typename T>
class UnexpectedError
	: public std::exception {
public:
	UnexpectedError(T const& unexpected)
		: unexpected(unexpected) {
	}
	T unexpected;
};

template<typename T>
class ExpectedError 
	: public std::exception {
public:
	ExpectedError(T const& expected, std::optional<T> found = std::nullopt)
		: expected(expected), found(found) {
	}
	T expected;
	std::optional<T> found;

};