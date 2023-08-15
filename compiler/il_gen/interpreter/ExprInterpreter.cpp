#include "ExprInterpreter.h"
#include <cassert>

using enum Token::Type;

ExprInterpreter::ExprInterpreter(Enviroment& env, TypeSystem& typeSystem)
	: env(env), typeSystem(typeSystem)
{
}

void ExprInterpreter::visit(Expr::Binary& expr)
{
	auto lhs_result = visitPtr(expr.lhs);
	auto rhs_result = visitPtr(expr.rhs);
	if (!isIntegral(lhs_result) || !isIntegral(rhs_result)) {
		throw NotConstEval(expr.sourcePos, expr.oper);
	}
	int lhs = std::get<u16>(lhs_result), rhs = std::get<u16>(rhs_result), retval;
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
		assert(false);
	}
	returnValue(static_cast<u16>(retval));
}

void ExprInterpreter::visit(Expr::Unary& expr)
{
	auto rhs_result = visitPtr(expr.expr);
	if (!isIntegral(rhs_result) || !isType(rhs_result)) {
		throw NotConstEval(expr.sourcePos, expr.oper);
	}
	if(isIntegral(rhs_result)){
		int rhs = std::get<u16>(rhs_result), retval;
		switch (expr.oper) {
		case MINUS:			retval = -rhs;	break;
		case BANG:			retval = !rhs;	break;
		case BIT_NOT:		retval = ~rhs;	break;
		case TYPE_DEREF:	assert(false);	break;
		default:
			throw InvalidOperation(expr.sourcePos, expr.oper, rhs_result);
		}
		returnValue(static_cast<u16>(retval));
	}
	else if (isType(rhs_result)) {
		auto& rhs = std::get<TypeSystem::TypeInstance>(rhs_result);
		switch (expr.oper) {
		case MUT:
			rhs.isMut = true;
		default:
			throw InvalidOperation(expr.sourcePos, expr.oper, rhs_result);
		}
		returnValue(std::move(rhs));
	}
}

void ExprInterpreter::visit(Expr::Parenthesis& expr)
{
	returnValue(visitPtr(expr.expr));
}

void ExprInterpreter::visit(Expr::Identifier& expr)
{
	if (typeSystem.isType(expr.ident)) {
		returnValue(TypeSystem::TypeInstance{ &typeSystem.getType(expr.ident), false });
	}
	else {
		throw NotConstEval(expr.sourcePos, "Variable lookup cannot be performed at compile-time");
	}
}

void ExprInterpreter::visit(Expr::FunctionCall& expr)
{
	throw NotConstEval(expr.sourcePos, "Function calls cannot be performed at compile-time");
}

void ExprInterpreter::visit(Expr::TemplateCall& expr)
{
}

void ExprInterpreter::visit(Expr::Indexing& expr)
{
	throw NotConstEval(expr.sourcePos, "Indexing is not a compile-time operation");
}

void ExprInterpreter::visit(Expr::MemberAccess& expr)
{
	throw NotConstEval(expr.sourcePos, "Member access is not a compile-time operation");
}

void ExprInterpreter::visit(Expr::Literal& expr)
{
	std::visit([&](auto&& arg) {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, std::string>) {
			returnValue(arg);
		}
		else if constexpr (std::is_same_v<U, u16>) {
			returnValue(arg);
		}
		else {
			NOT_REACHABLE;
		}
	}, expr.literal);
}

void ExprInterpreter::visit(Expr::Register& expr)
{
	throw NotConstEval(expr.sourcePos, "Registers are not compile-time constants");
}

void ExprInterpreter::visit(Expr::Flag& expr)
{
	throw NotConstEval(expr.sourcePos, "Flags are not compile-time constants");
}

void ExprInterpreter::visit(Expr::CurrentPC& expr)
{
	throw NotConstEval(expr.sourcePos, "The Program Counter is not a compile-time constant");
}

bool ExprInterpreter::isIntegral(ExprResult const& result)
{
	return std::holds_alternative<u16>(result);
}

bool ExprInterpreter::isType(ExprResult const& result)
{
	return std::holds_alternative<TypeSystem::TypeInstance>(result);
}

bool ExprInterpreter::isString(ExprResult const& result)
{
	return std::holds_alternative<std::string>(result);
}

