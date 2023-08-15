#include "ExprParser.h"



using enum Token::Type;

//expressions
Expr::UniquePtr ExprParser::expr()
{
	return logical();
}

Expr::UniquePtr ExprParser::logical()
{
	auto lhs = bitwise();
	while (matchType(AND, OR)) {
		Token::Type oper = previousType(); //order of function eval unspecified
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = bitwise();
		lhs = Expr::makeExpr<Expr::Logical>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::bitwise()
{
	auto lhs = comparison();
	while (matchType(BIT_AND, BIT_XOR, BIT_OR)) {
		Token::Type oper = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = comparison();
		lhs = Expr::makeExpr<Expr::Bitwise>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::comparison()
{
	auto lhs = equality();
	while (matchType(LESS, LESS_EQUAL, GREATER, GREATER_EQUAL)) {
		Token::Type oper = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = equality();
		lhs = Expr::makeExpr<Expr::Comparison>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::equality()
{
	auto lhs = bitshift();
	while (matchType(EQUAL, NOT_EQUAL)) {
		Token::Type oper = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = bitshift();
		lhs = Expr::makeExpr<Expr::Equality>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::bitshift()
{
	auto lhs = term();
	while (matchType(SHIFT_LEFT, SHIFT_RIGHT)) {
		Token::Type oper = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = term();
		lhs = Expr::makeExpr<Expr::Bitshift>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::term()
{
	auto lhs = factor();
	while (matchType(PLUS, MINUS)) {
		Token::Type oper = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = factor();
		lhs = Expr::makeExpr<Expr::Term>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::factor()
{
	auto lhs = unary();
	while (matchType(STAR, SLASH, MODULO)) {
		Token::Type oper = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		auto rhs = unary();
		lhs = Expr::makeExpr<Expr::Factor>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::unary()
{
	using enum Token::Type;
	if (matchType(MINUS, BANG, BIT_NOT, TYPE_DEREF)) {
		auto type = previousType();
		Token::SourcePosition sourcePos = previousSourcePos();
		return Expr::makeExpr<Expr::Unary>(sourcePos, type, unary());
	}
	else {
		return primary();
	}
}

Expr::UniquePtr ExprParser::primary()
{
	using enum Token::Type;
	if (matchType(LEFT_PARENTH)) {
		Token::SourcePosition sourcePos = previousSourcePos();
		auto parenth = Expr::makeExpr<Expr::Parenthesis>(sourcePos, expr());
		expect(RIGHT_PARENTH);
		return parenth;
	}
	else if (matchType(PESO)) {
		return Expr::makeExpr<Expr::CurrentPC>(previousSourcePos());
	}
	else if (matchType(IDENT)) {
		return Expr::makeExpr<Expr::Identifier>(previousSourcePos(), peekPrevious().lexeme);
	}
	else if (matchType(REGISTER)) {
		return Expr::makeExpr<Expr::Register>(previousSourcePos(), peekPrevious().lexeme);
	}
	else if (matchType(FLAG)) {
		return Expr::makeExpr<Expr::Flag>(previousSourcePos(), peekPrevious().lexeme);
	}
	else if (matchType(NUMBER, STRING)) {
		return Expr::makeExpr<Expr::Literal>(previousSourcePos(), peekPrevious().literal);
	}
	else {
		throw UnexpectedToken(peek());
	}
}

void ExprParser::expect(Token::Type expected)
{
	if (!matchType(expected)) {
		throw ExpectedTokenType(expected, peek());
	}
}

Token::Type ExprParser::previousType() const
{
	return peekPrevious().type;
}

Token::SourcePosition ExprParser::previousSourcePos() const
{
	return peekPrevious().sourcePos;
}
