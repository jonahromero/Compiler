#pragma once
#include <vector>
#include "MetaUtil.h"

namespace meta = util::meta;

template<typename T>
class ScopedContainer
{
public:
	template<bool IsConst>
	class basic_iterator
	{
		using ThisType = basic_iterator<IsConst>;
	public:
		using value_type = typename T::iterator::value_type;
		using reference = meta::conditional_const_t<IsConst, value_type>&;
		using pointer = meta::conditional_const_t<IsConst, value_type>*;

		ThisType& operator++()
		{
			++valueIterator;
			if (valueIterator == scopeIterator->end()) {
				++scopeIterator;
				valueIterator = scopeIterator->begin();
			}
			return *this;
		}

		ThisType operator++(int) {
			iterator temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(ThisType const& other) const
		{
			if (isEnd() || other.isEnd()) {
				return isEnd() == other.isEnd();
			}
			else
			{
				return scopeIterator == other.scopeIterator
					&& valueIterator == valueIterator;
			}
		}

		reference operator*() { return *valueIterator; }
		pointer operator->() { return &(*valueIterator); }
	private:
		friend class ScopedContainer<T>;
		using value_iterator_t = typename T::iterator;
		using scope_iterator_t = typename std::vector<T>::iterator;

		basic_iterator(scope_iterator_t scopeIterator, scope_iterator_t scopeIteratorEnd)
			: scopeIterator(scopeIterator), scopeIteratorEnd(scopeIteratorEnd) {}

		basic_iterator(scope_iterator_t scopeIterator, scope_iterator_t scopeIteratorEnd, value_iterator_t valueIterator)
			: valueIterator(valueIterator), scopeIterator(scopeIterator), scopeIteratorEnd(scopeIteratorEnd) {}

		template<bool IsOtherConst>
		basic_iterator(basic_iterator<IsOtherConst> const& other) 
			: valueIterator(other.valueIterator), 
			  scopeIterator(other.scopeIterator), 
			  scopeIteratorEnd(other.scopeIteratorEnd) {}

		bool isEnd() const { return scopeIterator == scopeIteratorEnd; }

		value_iterator_t valueIterator;
		scope_iterator_t scopeIterator, scopeIteratorEnd;
	};

	using const_iterator = basic_iterator<true>;
	using iterator = basic_iterator<false>;

	iterator find(typename iterator::value_type const& target) 
	{
		for (auto it = begin(); it != end(); ++it) {
			if (*it == target) 
				return it;
		}
		return end();
	}

	template<typename Callable>
	iterator find_if(Callable callable)
	{
		for (auto it = begin(); it != end(); ++it) {
			if (callable(std::as_const(*it)))
				return it;
		}
		return end();
	}


	iterator begin() {
		if (scopes.empty()) {
			return end();
		}
		else {
			return iterator(scopes.begin(), scopes.end(), scopes.front().begin());
		}
	}
	iterator end() {
		return iterator(scopes.end(), scopes.end());
	}
	const_iterator begin() const 
	{
		return const_cast<ScopedContainer<T>*>(this)->begin();
	}
	const_iterator end() const 
	{
		return const_cast<ScopedContainer<T>*>(this)->end();
	}

	T& newScope() { scopes.emplace_back(); return scopes.back(); }
	void destroyScope() { scopes.pop_back(); }
	T& scope(size_t scopeIndex) { return scopes[scopeIndex]; }
	T& currentScope() { return scopes.back(); }
private:
	std::vector<T> scopes;
};