#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "StringUtil.h"
#include "ArrayMap.h"
#include "TypeSystem.h"
#include "VariableCreator.h"
#include "ScopedContainer.h"
#include "IL.h"

class Enviroment 
{
public:
	Enviroment();

	void startScope();
	void destroyScope();

	// variables
	IL::Variable createAnonymousVariable(IL::Type ilType);
	void registerVariableName(std::string_view name, gen::Variable variable);

	// named variables
	bool isValidVariable(std::string_view name) const;
	gen::Variable const& getVariable(std::string_view name) const;

	// anonymous variables
	IL::Type getILVariableType(IL::Variable variable) const;

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