#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "StringUtil.h"
#include "Variable.h"
#include "ArrayMap.h"
#include "VariableCreator.h"
#include "ScopedContainer.h"
#include "TypeSystem.h"
#include "IL.h"

class Enviroment 
{
public:
	Enviroment();

	void newScope();
	void destroyScope();

	// creating variables
	IL::Variable createAnonymousVariable(IL::Type ilType);
	void registerVariableName(std::string_view name, gen::Variable variable);

	bool isValidVariable(std::string_view name) const;
	gen::Variable const& getVariable(std::string_view name) const;
	IL::Type getILVariableType(IL::Variable variable) const;

	// Type system
	TypeSystem types;

private:

	VariableCreator variableCreator;
	ScopedContainer<std::unordered_map<std::string_view, gen::Variable>> variables;
	ScopedContainer<std::unordered_map<IL::Variable, IL::Type>> ilVariableTypes;
};