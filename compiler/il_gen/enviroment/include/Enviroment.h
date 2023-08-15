#pragma once
#include <vector>
#include <unordered_map>
#include "StringUtil.h"
#include "ArrayMap.h"
#include "TypeSystem.h"
#include "VariableCreator.h"
#include "Generator.h"
#include "IL.h"

template<typename T>
class ScopedContainer
{
public:
	class iterator
	{
	public:
		using value_type = typename T::iterator::value_type;
		using reference = value_type&;
		using pointer = value_type*;

		iterator& operator++()
		{
			++valueIterator;
			if (valueIterator == scopeIterator->end()) {
				++scopeIterator;
				valueIterator = scopeIterator->begin();
			}
			return *this;
		}

		iterator operator++(int) {
			iterator temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(iterator const& other) const
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

		iterator(scope_iterator_t scopeIterator, scope_iterator_t scopeIteratorEnd)
			: scopeIterator(scopeIterator), scopeIteratorEnd(scopeIteratorEnd) {}

		iterator(scope_iterator_t scopeIterator, scope_iterator_t scopeIteratorEnd, value_iterator_t valueIterator)
			: valueIterator(valueIterator), scopeIterator(scopeIterator), scopeIteratorEnd(scopeIteratorEnd) {}

		bool isEnd() const { return scopeIterator == scopeIteratorEnd; }

		value_iterator_t valueIterator;
		scope_iterator_t scopeIterator, scopeIteratorEnd;
	};

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

	T& newScope() { scopes.emplace_back(); return scopes.back(); }
	void destroyScope() { scopes.pop_back(); }
	T& scope(size_t scopeIndex) { return scopes[scopeIndex]; }
	T& currentScope() { return scopes.back(); }
private:
	std::vector<T> scopes;
};

class Enviroment 
{
public:
	Enviroment();

	void startScope();
	void destroyScope();

	// variables
	IL::Variable createAnonymousVariable(IL::Type ilType);

	// named variables
	bool isValidVariable(std::string_view name) const;
	gen::Variable const& getVariable(std::string_view name) const;

	// anonymous variables
	IL::Type getILVariableType(IL::Variable variable) const;
	void upgradeILVariableToVariable(IL::Variable variable, std::string_view name, TypeInstance type, gen::ReferenceType type);

	// Type aliases
	bool isTypeAlias(std::string_view name) const;
	void addTypeAlias(std::string_view name, TypeInstance instance);
	TypeInstance const& getTypeAlias(std::string_view name) const;

	// Type system
	TypeSystem types;
	TypeInstance instantiateType(Expr::UniquePtr const& expr);

private:

	gen::Variable const* searchVariables(std::string_view name) const;
	TypeInstance const* searchTypeAliases(std::string_view name) const;

	VariableCreator variableCreator;

	ScopedContainer<std::unordered_map<std::string_view, gen::Variable>> variables;
	ScopedContainer<std::unordered_map<IL::Variable, IL::Type>> ilVariableTypes;
	ScopedContainer<std::unordered_map<std::string_view, TypeInstance>> typeAliases;

};