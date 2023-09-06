
#include <functional>
#include <tuple>

namespace util 
{
	template<typename T>
	struct FunctionTraits
		: FunctionTraits<decltype(&std::remove_reference_t<T>::operator())>
	{};
	template<typename Ret, typename...Args>
	struct FunctionTraits<Ret(Args...)>
	{
		template<size_t idx>
		using Arg = std::tuple_element_t<idx, std::tuple<Args...>>;
		using RetType = Ret;
	};

	template<typename Ret, typename...Args>
	struct FunctionTraits<Ret(*)(Args...)> 
		: FunctionTraits<Ret(Args...)> {
	};

	template<typename Ret, typename U, typename...Args>
	struct FunctionTraits<Ret (U::*)(Args...)> 
		: FunctionTraits<Ret(Args...)> {
	};

	template<typename Ret, typename U, typename...Args>
	struct FunctionTraits<Ret(U::*)(Args...) const> 
		: FunctionTraits<Ret(Args...)> {
	};
}