
#pragma once
#include "Enviroment.h"
#include "IL.h"

class FunctionEnviroment
	: public gen::Generator
{
public:
	FunctionEnviroment(Enviroment& env);
	FunctionEnviroment(FunctionEnviroment const&) = delete;
	FunctionEnviroment& operator=(FunctionEnviroment const&) = delete;
	~FunctionEnviroment();

	void addParameters(IL::Program& instructions, std::vector<Stmt::VarDecl> const& parameters);
	IL::Function::Signature determineILSignature(std::optional<gen::Variable> const& retType);

private:
	Enviroment& env;
	std::vector<gen::Variable> parameters;

	gen::Variable allocateParameter(IL::Program& instructions, std::string_view name, TypeInstance type);
	std::vector<IL::Decl> determineILParameters(std::optional<gen::Variable> const& returnVariable);
	IL::Type determineILReturnType(std::optional<gen::Variable> const& returnVariable);
};

class FunctionCaller
	: public gen::Generator
{
public:
	FunctionCaller(Enviroment& env, FunctionType const* functionType);
	gen::Variable callFunction(IL::Program& instructions, Expr::FunctionCall const& expr);

private:
	Enviroment& env;
	FunctionType const* functionType;

	void simpleCallFunction(IL::Program& instructions, IL::Variable returnValue, IL::FunctionCall::Callable callable, std::vector<IL::Value> args);
	IL::FunctionCall::Callable determineCallable(IL::Program& instructions, Expr::UniquePtr const& callable);
	std::vector<gen::Variable> allocateArguments(IL::Program& instructions, std::vector<Expr::UniquePtr> const& args);
};