
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
	typeAliases.newScope();
	variables.newScope();
	ilVariableTypes.newScope();
}

void Enviroment::destroyScope()
{
	typeAliases.destroyScope();
	variables.destroyScope();
	ilVariableTypes.destroyScope();
}

IL::Variable Enviroment::createAnonymousVariable(IL::Type ilType)
{
	auto temp = variableCreator.createVariable();
	ilVariableTypes.currentScope().emplace(temp, std::move(ilType));
	return temp;
}

bool Enviroment::isValidVariable(std::string_view name) const
{
	return searchVariables(name) != nullptr;
}

void Enviroment::upgradeILVariableToVariable(IL::Variable variable, std::string_view name, TypeInstance type, gen::ReferenceType refType)
{
	variables.currentScope().emplace(name,
		gen::Variable {
			variable, refType, type,
		}
	);
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
	typeAliases.currentScope().emplace(name, instance);
}

TypeInstance const& Enviroment::getTypeAlias(std::string_view name) const
{
	return *searchTypeAliases(name);
}

bool Enviroment::isTypeAlias(std::string_view name) const
{
	return searchTypeAliases(name) != nullptr;
}

gen::Variable const* Enviroment::searchVariables(std::string_view targetName) const
{
	// You're more likely to use variables in the immideate scope
	for (auto& [name, variable] : variables)
	{
		if (targetName == name) 
		{
			return &variable;
		}
	}
	return nullptr;
}

TypeInstance const* Enviroment::searchTypeAliases(std::string_view targetName) const
{
	for (auto& [name, type] : typeAliases)
	{
		if (targetName == name)
		{
			return &type;
		}
	}
	return nullptr;
}