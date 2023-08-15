#include "ExprInterpreter.h"
#include "SemanticError.h"
#include "StringUtil.h"
#include "VectorUtil.h"
#include "TemplateReplacer.h"
#include <algorithm>
#include <functional>
#include <cassert>

using enum Token::Type;

ExprInterpreter::ExprInterpreter(Enviroment& env)
	: env(env) {}

ComputedExpr ExprInterpreter::interpret(Expr::UniquePtr const& expr)
{
	return visitChild(expr);
}

void ExprInterpreter::visit(Expr::Binary& expr)
{
	auto lhs_result = visitChild(expr.lhs);
	auto rhs_result = visitChild(expr.rhs);
	if (!lhs_result.isInt() || !rhs_result.isInt()) {
		throw NotConstEvaluable(expr.sourcePos, expr.oper);
	}
	int retval, lhs = lhs_result.getInt(), rhs = rhs_result.getInt();
	switch (expr.oper) {
	case PLUS:			retval = lhs + rhs;	break;
	case MINUS:			retval = lhs - rhs;	break;
	case SLASH:			retval = lhs / rhs;	break;
	case STAR:			retval = lhs * rhs;	break;
	case MODULO:		retval = lhs % rhs;	break;
	case EQUAL_EQUAL:	retval = lhs == rhs; break;
	case NOT_EQUAL:		retval = lhs != rhs; break;
	case LESS_EQUAL:	retval = lhs <= rhs; break;
	case GREATER:		retval = lhs > rhs; break;
	case GREATER_EQUAL: retval = lhs >= rhs; break;
	case AND:			retval = lhs && rhs; break;
	case OR:			retval = lhs || rhs; break;
	case SHIFT_LEFT:	retval = lhs << rhs; break;
	case SHIFT_RIGHT:	retval = lhs >> rhs; break;
	case BIT_OR:		retval = lhs | rhs;	break;
	case BIT_AND:		retval = lhs & rhs;	break;
	default:
		COMPILER_NOT_REACHABLE;
	}
	returnValue(ComputedExpr{ expr.sourcePos, static_cast<u16>(retval) });
}

void ExprInterpreter::visit(Expr::Unary& expr)
{
	auto rhs_result = visitChild(expr.expr);
	if (!rhs_result.isInt() && !rhs_result.isTypeInstance()) {
		throw NotConstEvaluable(expr.sourcePos, expr.oper);
	}
	if(rhs_result.isInt()){
		int retval, rhs = rhs_result.getInt();
		switch (expr.oper) {
		case MINUS:			retval = -rhs;	break;
		case BANG:			retval = !rhs;	break;
		case BIT_NOT:		retval = ~rhs;	break;
		case TYPE_DEREF:	
			COMPILER_NOT_SUPPORTED; 
			COMPILER_NOT_REACHABLE;
		default:
			throw InvalidOperation(expr.sourcePos, expr.oper, rhs_result);
		}
		returnValue(ComputedExpr{ expr.sourcePos, static_cast<u16>(retval) });
	}
	else if (rhs_result.isTypeInstance()) {
		auto rhs = rhs_result.getTypeInstance();
		switch (expr.oper) {
		case MUT:
			if (rhs.isMut) {
				throw SemanticError(expr.sourcePos, "Only one mutable keyword required.");
			}
			rhs.isMut = true;
			break;
		default:
			throw InvalidOperation(expr.sourcePos, expr.oper, rhs_result);
		}
		returnValue(ComputedExpr{ expr.sourcePos, std::move(rhs) });
	}
}

void ExprInterpreter::visit(Expr::KeyworkFunctionCall& expr)
{
	switch (expr.function) 
	{
	case Token::Type::SIZEOF: 
		if (expr.args.size() != 1) {
			throw SemanticError(expr.sourcePos, "Sizeof operator expects exactly 1 argument.");
		}
		returnValue(ComputedExpr{ expr.sourcePos, u16(env.types.calculateTypeSize(env.instantiateType(expr.args[0]))) });
	case Token::Type::DEREF:
		throw NotConstEvaluable(expr.sourcePos, "Cannot perform a dereference at compile-time.");
	default:
		COMPILER_NOT_REACHABLE;
	}
}

void ExprInterpreter::visit(Expr::FunctionType& expr)
{
	auto params = util::transform_vector(expr.paramTypes, [&](Expr::UniquePtr const& param) {
		return env.instantiateType(param);
	});
	auto returnType = env.instantiateType(expr.returnType);
	auto returnTypeInstantiated = env.types.addFunction(name, std::move(params), std::move(returnType));
	returnValue(ComputedExpr{ expr.sourcePos, TypeInstance(returnTypeInstantiated) });
}

void ExprInterpreter::visit(Expr::Cast& expr)
{
	auto type = env.instantiateType(expr.type);
	auto result = visitChild(expr.expr);
	if (result.isInt()) {
		COMPILER_NOT_SUPPORTED;
	}
	else {
		throw SemanticError(expr.sourcePos, fmt::format("Unable to cast expression to: {}", type.type->name));
	}
}

void ExprInterpreter::visit(Expr::Parenthesis& expr)
{
	returnValue(visitChild(expr.expr));
}

void ExprInterpreter::visit(Expr::Identifier& expr)
{
	if (env.types.isType(expr.ident)) {
		returnValue(ComputedExpr{expr.sourcePos, TypeInstance(&env.types.getType(expr.ident))});
	}
	else if (env.isTypeAlias(expr.ident)) {
		returnValue(ComputedExpr{ expr.sourcePos, env.getTypeAlias(expr.ident) });
	}
	else {
		throw NotConstEvaluable(expr.sourcePos, "Variable lookup cannot be performed at compile-time");
	}
}

void ExprInterpreter::visit(Expr::FunctionCall& expr)
{
	throw NotConstEvaluable(expr.sourcePos, "Function calls cannot be performed at compile-time");
}

void ExprInterpreter::visit(Expr::TemplateCall& expr)
{
	TypeInstance lhs = env.instantiateType(expr.lhs);
	TemplateBin const* templateType = lhs.type->getExactType<TemplateBin>();
	if (!templateType) {
		throw SemanticError(expr.sourcePos,
			fmt::format("Tried to pass template arguments to an invalid expression. {} is not a template", lhs.type->name)
		);
	}
	auto computedArgs = util::transform_vector(expr.templateArgs, [&](Expr::UniquePtr const& expr) { return interpret(expr); });
	std::string_view name = compileTemplate(expr.sourcePos, templateType, std::move(computedArgs));
	returnValue(ComputedExpr{ expr.sourcePos, TypeInstance(& env.types.getType(name)) });
}

void ExprInterpreter::visit(Expr::Indexing& expr)
{
	throw NotConstEvaluable(expr.sourcePos, "Indexing is not a compile-time operation");
}

void ExprInterpreter::visit(Expr::MemberAccess& expr)
{
	throw NotConstEvaluable(expr.sourcePos, "Member access is not a compile-time operation");
}

void ExprInterpreter::visit(Expr::Questionable& expr)
{
	auto lhs = env.instantiateType(expr.expr);
	if (lhs.isOpt) {
		throw SemanticError(expr.sourcePos, "Cannot have an optional of an optional value.");
	}
	lhs.isOpt = true;
	returnValue(ComputedExpr{ expr.sourcePos, lhs });
}

void ExprInterpreter::visit(Expr::Reference& expr)
{
	auto lhs = env.instantiateType(expr.expr);
	if (lhs.isRef) {
		throw SemanticError(expr.sourcePos, "Multiple levels of referencing is not allowed.");
	}
	if (lhs.isOpt) {
		throw SemanticError(expr.sourcePos, "Cannot create build a reference of an optional type.");
	}
	lhs.isRef = true;
	returnValue(ComputedExpr{expr.sourcePos, lhs});
}

void ExprInterpreter::visit(Expr::Literal& expr)
{
	std::visit([&](auto&& arg) {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, u16>) 
		{
			returnValue(ComputedExpr{ expr.sourcePos, arg });
		}
		else {
			COMPILER_NOT_REACHABLE;
		}
	}, expr.literal);
}

void ExprInterpreter::visit(Expr::Register& expr)
{
	throw NotConstEvaluable(expr.sourcePos, "Registers are not compile-time constants");
}

void ExprInterpreter::visit(Expr::Flag& expr)
{
	throw NotConstEvaluable(expr.sourcePos, "Flags are not compile-time constants");
}

void ExprInterpreter::visit(Expr::CurrentPC& expr)
{
	throw NotConstEvaluable(expr.sourcePos, "The Program Counter is not a compile-time constant");
}

void ExprInterpreter::visit(Expr::ListLiteral& expr) 
{
	throw NotConstEvaluable(expr.sourcePos, "A List Literal is not a compile-time constant");
}

void ExprInterpreter::visit(Expr::StructLiteral& expr) 
{
	throw NotConstEvaluable(expr.sourcePos, "A Struct Literal is not a compile-time constant");
}

std::string_view ExprInterpreter::compileTemplate(SourcePosition pos, TemplateBin const* type, std::vector<ComputedExpr> args)
{
	std::string name = createTemplateName(type->name, args);
	if (env.types.isType(name)) {
		return env.types.getType(name).name;
	}
	assertCorrectTemplateArgs(pos, type->templateParams, args);
	env.types.addBin(name, newBinBody(type->body, type->templateParams, args));
	return env.types.getType(name).name;
}

std::string ExprInterpreter::createTemplateName(std::string_view templateID, std::vector<ComputedExpr> const& args)
{
	std::string templateName = std::string{ templateID };
	templateName += '<';
	bool first = true;
	for (auto& arg : args) {
		if (!first) { templateName += ','; first = false; }
		templateName += arg.toString();
	}
	templateName += '>';
	return templateName;
}

void ExprInterpreter::assertCorrectTemplateArgs
(
	SourcePosition sourcePos,
	std::vector<TemplateBin::TemplateParam> const& params, 
	std::vector<ComputedExpr> const& args
)
{
	if (params.size() != args.size()) {
		throw SemanticError(sourcePos,
			fmt::format("Passed {} arguments to template, but expected {} arguments",
				args.size(), params.size())
		);
	}
	for (size_t i = 0; i < params.size(); i++)
	{
		std::visit([&](auto&& paramType) {
			using U = std::remove_cvref_t<decltype(paramType)>;
			if constexpr (std::is_same_v<U, TypeInstance>)
			{
				if (!args[i].isInt()) {
					throw SemanticError(args[i].sourcePos,
						fmt::format("Unable to assign a {} to the {} template argument expecting a {}",
							args[i].typeToString(), util::toStringWithOrdinalSuffix(i), paramType.type->name)
					);
				}
			}
			else if constexpr (std::is_same_v<U, TemplateBin::TypeParam>) {
				if (!args[i].isTypeInstance()) {
					throw SemanticError(args[i].sourcePos,
						fmt::format("Unable to convert a {} to the {} template argument expecting a type",
							args[i].typeToString(), util::toStringWithOrdinalSuffix(i))
					);
				}
			}
		}, params[i].type);
	}
}

std::vector<Stmt::VarDecl> ExprInterpreter::newBinBody(
	std::vector<Stmt::VarDecl> const& oldBody, 
	std::vector<TemplateBin::TemplateParam> const& params, 
	std::vector<ComputedExpr> const& args
) 
{
	std::vector<ExprTemplateReplacer> replacers;// {oldBody.begin(), oldBody.end()};
	std::vector<Stmt::VarDecl> newDecls;
	newDecls.reserve(oldBody.size());
	for (auto& oldDecl : oldBody) {
		newDecls.emplace_back(oldDecl.name, Expr::Cloner().clone(oldDecl.type)); 
		replacers.emplace_back(newDecls.back().type);
	}

	for (size_t i = 0; i < params.size(); i++)
	{
		std::string_view replacementName = params[i].name;
		Expr::UniquePtr replacementExpr = args[i].toExpr();
		for (auto& replacer : replacers) {
			replacer.replace(replacementName, replacementExpr);
		}
	}

	return newDecls;
}