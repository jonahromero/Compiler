#include "FunctionHelpers.h"
#include "ILExprResult.h"
#include "ExprGenerator.h"

FunctionEnviroment::FunctionEnviroment(Enviroment& env)
	: gen::GeneratorToolKit(env), env(env)
{
	env.newScope();
}

FunctionEnviroment::~FunctionEnviroment()
{
	env.destroyScope();
}

std::vector<TypeInstance> FunctionEnviroment::addParameters(IL::Program& instructions, std::vector<Stmt::VarDecl> const& parameterDecls)
{
	std::vector<TypeInstance> types;
	parameters = util::transform_vector(parameterDecls,
	[&](Stmt::VarDecl const& decl)
	{
		auto type = env.types.instantiateType(decl.type);
		types.push_back(type);
		return allocateParameter(instructions, decl.name, type);
	});
	return types;
}

IL::Function::Signature FunctionEnviroment::determineILSignature(std::optional<gen::Variable> const& returnVariable)
{
	IL::Function::Signature signature;
	signature.params = determineILParameters(returnVariable);
	signature.returnType = determineILReturnType(returnVariable);
	return signature;
}

gen::Variable FunctionEnviroment::allocateParameter(IL::Program& instructions, std::string_view name, TypeInstance type)
{
	gen::Variable parameter = allocateNonPossessingVariable(instructions, type);
	env.registerVariableName(name, parameter);
	return parameter;
}

std::vector<IL::Decl> FunctionEnviroment::determineILParameters(std::optional<gen::Variable> const& returnVariable)
{
	std::vector<IL::Decl> signature = util::transform_vector(parameters, [&](gen::Variable const& param) {
		return IL::Decl(param.ilName, env.getILVariableType(param.ilName));
	});

	if (shouldPassReturnAsParameter(returnVariable.value().type))
	{
		IL::Variable returnIlName = returnVariable.value().ilName;
		signature.push_back(IL::Decl(returnIlName, env.getILVariableType(returnIlName)));
	}
	return signature;
}

IL::Type FunctionEnviroment::determineILReturnType(std::optional<gen::Variable> const& returnVariable)
{
	if (!returnVariable.has_value() || shouldPassReturnAsParameter(returnVariable.value().type))
	{
		return IL::Type::void_;
	}
	else
	{
		return env.getILVariableType(returnVariable.value().ilName);
	}
}

FunctionCaller::FunctionCaller(Enviroment& env, FunctionType const* functionType)
	: gen::GeneratorToolKit(env), env(env), functionType(functionType)
{
}

gen::Variable FunctionCaller::callFunction(IL::Program& instructions, Expr::FunctionCall const& expr)
{
	IL::FunctionCall::Callable callable = determineCallable(instructions, expr.lhs);
	std::vector<gen::Variable> args = allocateArguments(instructions, expr.arguments);
	gen::Variable returnVariable = allocateVariable(instructions, functionType->returnType);

	std::vector<IL::Value> ilArgs = util::transform_vector(args, [&](gen::Variable const& arg) {
		return IL::Value(createBinding(instructions, arg).ilName);
	});

	if (shouldPassReturnAsParameter(functionType->returnType))
	{
		ilArgs.push_back(createBinding(instructions, returnVariable).ilName);
	}
	simpleCallFunction(instructions, returnVariable.ilName, callable, std::move(ilArgs));
	return returnVariable;
}

void FunctionCaller::simpleCallFunction(IL::Program& instructions, IL::Variable returnValue, IL::FunctionCall::Callable callable, std::vector<IL::Value> args)
{
	auto dest = shouldPassReturnAsParameter(functionType->returnType) ?
		IL::Decl(env.createAnonymousVariable(IL::Type::void_), IL::Type::void_) :
		IL::Decl(returnValue, env.getILVariableType(returnValue));
		instructions.push_back(IL::makeIL<IL::FunctionCall>(dest, callable, std::move(args)));
}

IL::FunctionCall::Callable FunctionCaller::determineCallable(IL::Program& instructions, Expr::UniquePtr const& callable)
{
	ILExprResult result = ExprGenerator::defaultContext(env).generate(callable);
	util::vector_append(instructions, std::move(result.instructions));
	if (result.isNamed())
		return result.getName();
	else
		return result.output.ilName;
}

std::vector<gen::Variable> FunctionCaller::allocateArguments(IL::Program& instructions, std::vector<Expr::UniquePtr> const& args)
{
	auto& paramTypes = functionType->params;
	return util::transform_vector(args, paramTypes,
		[&](Expr::UniquePtr const& arg, TypeInstance const& type) {
			auto argResult = ExprGenerator::typedContext(env, type.type).generate(arg);
			util::vector_append(instructions, std::move(argResult.instructions));
			return argResult.output;
		});
}
