
#pragma once
#include <type_traits>
#include <memory>

namespace util::meta 
{
	namespace detail 
	{
		template<typename T>
		struct IsUniquePtr : std::false_type {};

		template<typename T>
		struct IsUniquePtr<std::unique_ptr<T>> : std::true_type {};
	}
	
	template<bool cond, typename T>
	using conditional_const_t = std::conditional_t<cond, const T, T>;
	
	template<typename T>
	constexpr bool is_unique_ptr_v = detail::IsUniquePtr<T>::value;

}