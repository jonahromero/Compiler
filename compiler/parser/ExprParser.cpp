#include "ExprParser.h"

using enum Token::Type;

//expressions
Expr::UniquePtr ExprParser::expr()
{
	auto typeMode = context.turnOffTypeMode();
	auto scopedChange = context.startNewNesting();
	return nestedExpr();
}

Expr::UniquePtr ExprParser::typeExpr() {
	auto typeMode = context.turnOnTypeMode();
	auto scopedChange = context.startNewNesting();
	return nestedExpr();
}

Expr::UniquePtr ExprParser::nestedExpr()
{
	return logical();
}

Expr::UniquePtr ExprParser::logical()
{
	auto lhs = bitwise();
	while (matchType(OR) || (!context.isInTypeMode() && matchType(AND))) {
		Token::Type oper = previousType(); //order of function eval unspecified
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = bitwise();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::bitwise()
{
	auto lhs = comparison();
	while (matchType(BIT_XOR, BIT_OR)
		|| (!context.isInTypeMode() && matchType(BIT_AND))) {
		Token::Type oper = previousType();
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = comparison();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::comparison()
{
	auto lhs = equality();

	while (matchType(LESS_EQUAL, GREATER_EQUAL) 
		|| (context.isNested() && matchType(GREATER))
		|| (!context.isInTemplateMode() && matchType(GREATER))
		|| (!shouldMatchTemplate() && matchType(LESS))
	) 
	{
		Token::Type oper = previousType();
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = equality();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::equality()
{
	auto lhs = bitshift();
	while (matchType(EQUAL_EQUAL, NOT_EQUAL)) {
		Token::Type oper = previousType();
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = bitshift();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::bitshift()
{
	auto lhs = term();
	auto matchShiftRight = [&]() { 
		if (peekNext().type == GREATER_CONCATENATOR) { 
			expect(GREATER);
			matchType(GREATER_CONCATENATOR);
			expect(GREATER);
			return true;
		} 
		return false;
	};
	while (matchType(SHIFT_LEFT)
		|| (context.isNested() && matchShiftRight())
		|| (!context.isInTemplateMode() && matchShiftRight())
		) 
	{
		Token::Type oper = previousType() == GREATER ? SHIFT_RIGHT : SHIFT_LEFT;
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = term();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::term()
{
	auto lhs = factor();
	while (matchType(PLUS, MINUS)) {
		Token::Type oper = previousType();
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = factor();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::factor()
{
	auto lhs = cast();
	while (matchType(STAR, SLASH, MODULO)) {
		Token::Type oper = previousType();
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = cast();
		lhs = Expr::makeExpr<Expr::Binary>(sourcePos, std::move(lhs), oper, std::move(rhs));
	}
	return lhs;
}

Expr::UniquePtr ExprParser::cast()
{
	auto expr = unary();
	while (matchType(AS)) {
		SourcePosition sourcePos = previousSourcePos();
		auto type = unary();
		expr = Expr::makeExpr<Expr::Cast>(sourcePos, std::move(expr), std::move(type));
	}
	return expr;
}

Expr::UniquePtr ExprParser::unary()
{
	using enum Token::Type;
	if (matchType(MINUS, BANG, BIT_NOT, TYPE_DEREF, MUT, BIT_AND, AND)) {
		auto type = previousType();
		SourcePosition sourcePos = previousSourcePos();
		auto rhs = unary();
		if (type == AND) 
		{
			type = BIT_AND;
			rhs = Expr::makeExpr<Expr::Unary>(sourcePos, type, std::move(rhs));
			sourcePos.pos++;
		}
		return Expr::makeExpr<Expr::Unary>(sourcePos, type, std::move(rhs));
	}
	else {
		return scripts();
	}
}

Expr::UniquePtr ExprParser::scripts() 
{
	auto lhs = primary();
	while (matchType(PERIOD, LEFT_BRACKET, LEFT_PARENTH, LESS, QUESTION_MARK, BIT_AND, AND))
	{
		switch (previousType()) 
		{
		case PERIOD:
			do {
				lhs = Expr::makeExpr<Expr::MemberAccess>(previousSourcePos(), std::move(lhs), expectIdent());
			} while (matchType(PERIOD));
			break;
		case LEFT_BRACKET: {
			auto scopedChange = context.turnOffTemplateMode();
			auto innerExpr = nestedExpr();
			expect(RIGHT_BRACKET);
			lhs = Expr::makeExpr<Expr::Indexing>(previousSourcePos(), std::move(lhs), std::move(innerExpr));
			break;
		}
		case LEFT_PARENTH: {
			auto scopedChange = context.turnOffTemplateMode();
			std::vector<Expr::UniquePtr> args = argList(RIGHT_PARENTH);
			lhs = Expr::makeExpr<Expr::FunctionCall>(previousSourcePos(), std::move(lhs), std::move(args));
			break;
		}
		case LESS: {
			auto scopedChange = context.turnOnTemplateMode();
			Stmt::ArgList args = argList(GREATER);
			matchType(GREATER_CONCATENATOR);
			lhs = Expr::makeExpr<Expr::TemplateCall>(previousSourcePos(), std::move(lhs), std::move(args));
			break;
		}
		case ARROW: {
			auto returnType = scripts();

		}
		case QUESTION_MARK: {
			lhs = Expr::makeExpr<Expr::Questionable>(previousSourcePos(), std::move(lhs));
			break;
		}
		case BIT_AND: {
			lhs = Expr::makeExpr<Expr::Reference>(previousSourcePos(), std::move(lhs));
			break;
		}
		case AND: {
			lhs = Expr::makeExpr<Expr::Reference>(previousSourcePos(), std::move(lhs));
			lhs = Expr::makeExpr<Expr::Reference>(previousSourcePos(), std::move(lhs));
			break;
		}
		default:
			COMPILER_NOT_REACHABLE;
		}
	}
	return lhs;
}

Expr::UniquePtr ExprParser::primary()
{
	using enum Token::Type;
	if (matchType(LEFT_PARENTH)) {
		auto scopedChange = context.increaseNesting();
		SourcePosition sourcePos = previousSourcePos();
		auto args = argList(RIGHT_PARENTH);
		if (matchType(ARROW)) {
			auto returnType = scripts();
			return Expr::makeExpr<Expr::FunctionType>(sourcePos, std::move(args), std::move(returnType));
		}
		else {
			if (args.size() != 1) {
				throw ParseError(sourcePos, "Parenthesis does not support multiple expressions to be listed.");
			}
			return Expr::makeExpr<Expr::Parenthesis>(sourcePos, std::move(args[0]));
		}
	}
	else if (matchType(PESO)) {
		return Expr::makeExpr<Expr::CurrentPC>(previousSourcePos());
	}
	else if (matchType(IDENT)) {
		auto identifier = peekPrevious().lexeme;
		return Expr::makeExpr<Expr::Identifier>(previousSourcePos(), identifier);
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
	else if (matchType(SIZEOF, REF)) {
		auto pos = previousSourcePos();
		auto function = previousType();
		expect(LEFT_PARENTH);
		auto args = argList(RIGHT_PARENTH);
		return Expr::makeExpr<Expr::KeyworkFunctionCall>(pos, function, std::move(args));
	}
	else if (matchType(LEFT_BRACE)) {
		auto scopedChange = context.turnOffTemplateMode();
		auto pos = previousSourcePos();
		auto throwMismatchedStruct = [&]() { throw ParseError(pos, "Structure Literal must either be "); };
		auto canMatchName = [&]() { return peek().type == IDENT && peekNext().type == COLON; };
		bool isNamed = canMatchName();
		std::vector<Expr::UniquePtr> initializers;
		std::optional<std::vector<std::string_view>> names;
		do {
			if (!isNamed && canMatchName()) {
				throwMismatchedStruct();
			}
			if (isNamed) {
				if (!canMatchName()) {
					throwMismatchedStruct();
				}
				if (!names.has_value()) 
					names = std::vector<std::string_view>{};
				names.value().push_back(expectIdent());
				matchType(COLON);
			}
			initializers.push_back(nestedExpr());
		} while (matchType(COMMA));
		expect(RIGHT_BRACE);
		return Expr::makeExpr<Expr::StructLiteral>(pos, std::move(initializers), std::move(names));
	}
	else if (matchType(LEFT_BRACKET))
	{
		auto scopedChange = context.turnOffTemplateMode();
		auto pos = previousSourcePos();
		std::vector<Expr::UniquePtr> elements;
		do {
			elements.push_back(nestedExpr());
		} while (matchType(COMMA));
		expect(RIGHT_BRACKET);
		return Expr::makeExpr<Expr::ListLiteral>(pos, std::move(elements));
	}
	else {
		throw UnexpectedToken(peek());
	}
}

bool ExprParser::shouldMatchTemplate() const
{
	auto& prev = peekPrevious();
	return prev.type == IDENT && context.isTemplate(prev.lexeme);
}

void ExprParser::expect(Token::Type expected)
{
	if (!matchType(expected)) {
		throw ExpectedTokenType(expected, peek());
	}
}

auto ExprParser::expectIdent() -> std::string_view
{
	expect(IDENT);
	return peekPrevious().lexeme;
}

auto ExprParser::argList(Token::Type terminator) -> Stmt::ArgList
{
	Stmt::ArgList retval;
	if (matchType(terminator)) {
		return retval;
	}
	do {
		retval.push_back(expr());
	} while (matchType(COMMA));
	expect(terminator);
	return retval;
}

Token::Type ExprParser::previousType() const
{
	return peekPrevious().type;
}

SourcePosition ExprParser::previousSourcePos() const
{
	return peekPrevious().sourcePos;
}
