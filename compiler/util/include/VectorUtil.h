#pragma once
#include <vector>
#include "FunctionUtil.h"

namespace util {
	namespace detail {
		template<typename T>
		struct VectorType {
			using type = T;
			static constexpr bool is_vector = false;
		};

		template<typename U>
		struct VectorType<std::vector<U>> {
			using type = U;
			static constexpr bool is_vector = true;
		};

		template<typename T>
		auto elems_in_vector(T const& vec) -> size_t {
			if constexpr (VectorType<std::remove_cvref_t<T>>::is_vector) {
				return vec.size();
			}
			else {
				return 1;
			}
		}
	}

	template<typename T, typename U>
	void vector_append(std::vector<T>& vec, U&& value) {
		if constexpr (detail::VectorType<std::remove_cvref_t<U>>::is_vector) {
			if constexpr (std::is_same_v<U, std::remove_cvref_t<U&&>>) {
				vec.insert(vec.end(),
					std::make_move_iterator(value.begin()),
					std::make_move_iterator(value.end())
				);
			}
			else {
				vec.insert(vec.end(), value.begin(), value.end());
			}
		}
		else {
			vec.emplace_back(std::forward<U>(value));
		}
	}
	template<typename T, typename...Args>
	auto combine_vectors(T&& first, Args&&...rest) {
		std::vector<typename detail::VectorType<std::remove_cvref_t<T>>::type> retval;
		if constexpr (sizeof...(rest) == 0) {
			retval.reserve(detail::elems_in_vector(first));
		}
		else {
			retval.reserve(detail::elems_in_vector(first) + (detail::elems_in_vector(rest) + ...));
		}
		vector_append(retval, std::forward<T>(first));
		(vector_append(retval, std::forward<Args>(rest)), ...);
		return retval;
	}

	template<typename T, typename...Args>
	auto create_vector(Args&&...args)->std::vector<T> {
		std::vector<T> retval; retval.reserve(sizeof...(args));
		(retval.push_back(std::forward<Args>(args)), ...);
		return retval;
	}

	template<typename T, typename Callable, typename Ret = std::invoke_result_t<Callable, T const&>>
	std::vector<Ret> transform_vector(std::vector<T> const& vec, Callable callable)
	{
		std::vector<Ret> retval;
		retval.reserve(vec.size());
		for (auto& elem : vec) {
			retval.emplace_back(callable(elem));
		}
		return retval;
	}

	template<typename T, typename U, typename Callable, typename Ret = std::invoke_result_t<Callable, T const&, U const&>>
	auto transform_vector(std::vector<T> const& first, std::vector<U> const& second, Callable callable) {
		std::vector<Ret> retval;
		assert(first.size() == second.size());
		retval.reserve(first.size());
		for (size_t i = 0; i < first.size(); i++) {
			retval.push_back(callable(first[i], second[i]));
		}
		return retval;
	}
}