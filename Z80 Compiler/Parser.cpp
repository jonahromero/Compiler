#include "Parser.h"
#include "Expr.h"
#include "Logger.h"

using enum Token::Type;

Parser::Program Parser::program()
{	
	Program program;
	while (!matchType(EOF_)) {
		tryToAddStmt(program);
	}
	return program;
}

Stmt::UniquePtr Parser::stmt()
{
	if (matchType(OPCODE)) {
		auto opcode = peekPrevious().lexeme;
		return Stmt::makeStmt<Stmt::Instruction>(opcode, argList());;
	}
	else if (matchType(IDENT)) {
		auto ident = peekPrevious().lexeme;
		expectType(COLON);
		return Stmt::makeStmt<Stmt::Label>(ident);
	}
	else if (matchType(NEWLINE, BACKSLASH)) {
		return Stmt::makeStmt<Stmt::NullStmt>();
	}
	else {
		unexpectedElement(peek());
	}
}

Expr::UniquePtr Parser::expr()
{
	return logical();
}


void Parser::tryToAddStmt(Program& program)
{
	try {
		program.push_back(stmt());
		return;
	}
	catch(ExpectedType const& except) {
		auto msg = "Expected to find: " + std::string(tokenTypeToStr(except.expected)) + 
					", but found" + std::string(tokenTypeToStr(except.found.value())) + "\n";
		Logger::log(msg);
	}
	catch (UnexpectedElement const& except) {
		auto msg = "Unexpected Element found: " + std::string(tokenTypeToStr(except.unexpected.type)) + "\n";
		Logger::log(msg);
	}
	synchronize();
}

void Parser::synchronize()
{
	while (!matchStmtTerminator()) {
		advance();
	}
}

//statements
Stmt::ArgList Parser::argList()
{
	Stmt::ArgList retval;
	if (!matchStmtTerminator()) {
		retval.push_back(expr());
		while (!matchStmtTerminator()) {
			expectType(COMMA);
			retval.push_back(expr());
		}
	}
	return retval;
}

bool Parser::matchStmtTerminator()
{
	//we dont consume end of file
	if (peek().type == EOF_) return true;
	return matchType(NEWLINE, BACKSLASH);
}

//expressions
Expr::UniquePtr Parser::logical()
{
	auto lhs = bitwise();
	while (matchType(AND, OR)) {
		Token::Type oper = previousType(); //order of function eval unspecified
		auto rhs = bitwise();
		lhs = Expr::makeExpr<Expr::Logical>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::bitwise()
{
	auto lhs = comparison();
	while (matchType(BIT_AND, BIT_XOR, BIT_OR)) {
		Token::Type oper = previousType();
		auto rhs = comparison();
		lhs = Expr::makeExpr<Expr::Bitwise>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::comparison()
{
	auto lhs = equality();
	while (matchType(LESS, LESS_EQUAL, GREATER, GREATER_EQUAL)) {
		Token::Type oper = previousType();
		auto rhs = equality();
		lhs = Expr::makeExpr<Expr::Comparison>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::equality()
{
	auto lhs = bitshift();
	while (matchType(EQUAL, NOT_EQUAL)) {
		Token::Type oper = previousType();
		auto rhs = bitshift();
		lhs = Expr::makeExpr<Expr::Equality>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::bitshift()
{
	auto lhs = term();
	while (matchType(SHIFT_LEFT, SHIFT_RIGHT)) {
		Token::Type oper = previousType();
		auto rhs = term();
		lhs = Expr::makeExpr<Expr::Bitshift>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::term()
{
	auto lhs = factor();
	while (matchType(PLUS, MINUS)) {
		Token::Type oper = previousType();
		auto rhs = factor();
		lhs = Expr::makeExpr<Expr::Term>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::factor()
{
	auto lhs = unary();
	while (matchType(STAR, SLASH, MODULO)) {
		Token::Type oper = previousType();
		auto rhs = unary();
		lhs = Expr::makeExpr<Expr::Factor>(std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr Parser::unary()
{
	using enum Token::Type;
	if (matchType(MINUS, BANG, BIT_NOT)) {
		auto type = previousType(); 
		return Expr::makeExpr<Expr::Unary>(type, unary());
	}
	else {
		return primary();
	}
}

Expr::UniquePtr Parser::primary()
{
	using enum Token::Type;
	if (matchType(LEFT_PARENTH)) {
		auto parenth = Expr::makeExpr<Expr::Parenthesis>(expr());
		expectType(RIGHT_PARENTH);
		return parenth;
	}
	else if (matchType(PESO)) {
		return Expr::makeExpr<Expr::CurrentPC>();
	}
	else if (matchType(IDENT)) {
		return Expr::makeExpr<Expr::Identifier>(peekPrevious().lexeme);
	}
	else if (matchType(REGISTER)) {
		return Expr::makeExpr<Expr::Register>(peekPrevious().lexeme);
	}
	else if (matchType(FLAG)) {
		return Expr::makeExpr<Expr::Flag>(peekPrevious().lexeme);
	}
	else if (matchType(NUMBER, STRING)) {
		return Expr::makeExpr<Expr::Literal>(peekPrevious().literal);
	}
	else {
		unexpectedElement(peek());
	}
}

Token::Type Parser::previousType()
{
	return peekPrevious().type;
}

