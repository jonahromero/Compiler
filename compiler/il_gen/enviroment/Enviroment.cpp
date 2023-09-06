
#include "Enviroment.h"
#include "SemanticError.h"
#include <ranges>

Enviroment::Enviroment()
{
	// add the default global scope
	newScope();
}

void Enviroment::newScope()
{
	types.newScope();
	variables.newScope();
	ilVariableTypes.newScope();
}

void Enviroment::destroyScope()
{
	types.destroyScope();
	variables.destroyScope();
	ilVariableTypes.destroyScope();
}

IL::Variable Enviroment::createAnonymousVariable(IL::Type ilType)
{
	auto temp = variableCreator.createVariable();
	ilVariableTypes.currentScope().emplace(temp, std::move(ilType));
	return temp;
}

bool Enviroment::isValidVariable(std::string_view targetName) const
{
	return variables.find_if([&](auto& pair) {
		return pair.first == targetName;
	}) != variables.end();
}

void Enviroment::registerVariableName(std::string_view name, gen::Variable variable)
{
	COMPILER_ASSERT("IL variable must already exist", ilVariableTypes.find_if([&](auto& pair) {
		return pair.first == variable.ilName;
	}) != ilVariableTypes.end());

	variables.currentScope().emplace(name, variable);
}

gen::Variable const& Enviroment::getVariable(std::string_view name) const
{
	return variables.find_if([&](auto& pair) {
		return pair.first == name;
	})->second;
}

IL::Type Enviroment::getILVariableType(IL::Variable variable) const
{
	return ilVariableTypes.find_if([&](auto& pair) {
		return pair.first == variable;
	})->second;
}
