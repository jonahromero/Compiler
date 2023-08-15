
#include "Enviroment.h"
#include "ExprInterpreter.h"
#include "SemanticError.h"
#include <ranges>

Enviroment::Enviroment()
	: types(*this)
{
	// add the default global scope
	startScope();
}

void Enviroment::startScope()
{
	typeAliases.emplace_back();
	variables.emplace_back();
	ilAliasTypes.emplace_back();
}

void Enviroment::endScope()
{
	typeAliases.pop_back();
	variables.pop_back();
	ilAliasTypes.pop_back();
}

IL::Variable Enviroment::createVariable(std::string_view name, TypeInstance type, IL::ReferenceType refType)
{
	auto alias = createAnonymousVariable(types.compileType(type.type));
	variables.back().emplace(name, VarInfo{ type, alias, refType });
	return alias;
}

IL::Variable Enviroment::createAnonymousVariable(IL::Type ilType)
{
	auto temp = variableCreator.createVariable();
	ilAliasTypes.back().emplace(temp, std::move(ilType));
	return temp;
}

bool Enviroment::isValidVariable(std::string_view name) const
{
	return searchVariables(name) != nullptr;
}

IL::Variable const& Enviroment::getVariableILAlias(std::string_view name) const
{
	return searchVariables(name)->ilAlias;
}

TypeInstance const& Enviroment::getVariableType(std::string_view name) const
{
	return searchVariables(name)->type;
}

IL::ReferenceType Enviroment::getVariableReferenceType(std::string_view name) const
{
	return searchVariables(name)->refType;
}

IL::Type Enviroment::getILAliasType(IL::Variable variable) const
{
	return ilAliasTypes.back().at(variable);
}

TypeInstance Enviroment::instantiateType(Expr::UniquePtr const& expr)
{
	try {
		ComputedExpr result = ExprInterpreter{ *this }.interpret(expr);
		if (!result.isTypeInstance()) {
			throw SemanticError(
				expr->sourcePos, 
				fmt::format("Expected a type, but found a {} instead", result.typeToString())
			);
		}
		return result.getTypeInstance();
	}
	catch (ExprInterpreter::NotConstEvaluable const&) {
		throw SemanticError(expr->sourcePos, "Expected a type, but was unable to determine the expression at compile time");
	}
}

void Enviroment::addTypeAlias(std::string_view name, TypeInstance instance)
{
	typeAliases.back().emplace(name, instance);
}

TypeInstance const& Enviroment::getTypeAlias(std::string_view name) const
{
	return *searchTypeAliases(name);
}

bool Enviroment::isTypeAlias(std::string_view name) const
{
	return searchTypeAliases(name) != nullptr;
}

Enviroment::VarInfo const* Enviroment::searchVariables(std::string_view name) const
{
	// You're more likely to use variables in the immideate scope
	for (auto mapping = variables.rbegin(); mapping != variables.rend(); mapping++)
	{
		if (auto it = mapping->find(name); it != mapping->end()) {
			return &(it->second);
		}
	}
	return nullptr;
}

TypeInstance const* Enviroment::searchTypeAliases(std::string_view name) const
{
	for (auto mapping = typeAliases.rbegin(); mapping != typeAliases.rend(); mapping++)
	{
		if (auto it = mapping->find(name); it != mapping->end()) {
			return &(it->second);
		}
	}
	return nullptr;
}