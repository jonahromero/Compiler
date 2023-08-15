#pragma once
#include <vector>
#include <span>
#include <array>
#include <stack>
#include <unordered_set>
#include "Expr.h"
#include "Stmt.h"
#include "TokenViewer.h"
#include "ParseError.h"
#include "ParserContext.h"

class ExprParser
	: public TokenViewer
{
public:
	ExprParser(std::span<const Token> tokens, ParserContext& context)
		: TokenViewer(tokens.data(), tokens.size()), context(context) {
	}

	Expr::UniquePtr expr();
	Expr::UniquePtr typeExpr();
private:
	Expr::UniquePtr nestedExpr();
	Expr::UniquePtr logical();
	Expr::UniquePtr bitwise();
	Expr::UniquePtr comparison();
	Expr::UniquePtr equality();
	Expr::UniquePtr bitshift();
	Expr::UniquePtr term();
	Expr::UniquePtr factor();
	Expr::UniquePtr cast();
	Expr::UniquePtr unary();
	Expr::UniquePtr scripts();
	Expr::UniquePtr primary();

protected:
	ParserContext& context;
	bool shouldMatchTemplate() const;

	Token::Type previousType() const;
	SourcePosition previousSourcePos() const;

	void expect(Token::Type expected);
	auto expectIdent()->std::string_view;
	auto argList(Token::Type terminator)-> Stmt::ArgList;

	template<typename...Args>
	void expectConsecutive(Args...args)
	{
		std::array<Token::Type, sizeof...(args)> expected = { {args...} };
		for (auto& expectation : expected) {
			if (!matchType(expectation)) {
				throw ExpectedTokenType(expectation, peek());
			}
		}
	}

	class ExpectedTokenType : public ParseError {
	public:
		ExpectedTokenType(Token::Type const& expectedType, Token const& found)
			: ParseError(found.sourcePos, createMessage(expectedType, found)) {}

	private:
		static std::string createMessage(Token::Type const& expectedType, Token const& found) {
			if (found.type == Token::Type::EOF_) {
				return fmt::format("Expected a token of type: {}, but reached the end of the file.",
					tokenTypeToStr(expectedType));
			}
			else {
				return fmt::format("Expected a token of type: {}, but found a {}.",
					tokenTypeToStr(expectedType), tokenTypeToStr(found.type));
			}
		}
	};

	class UnexpectedToken : public ParseError 
	{
	public:
		UnexpectedToken(Token const& unexpected)
			: ParseError(unexpected.sourcePos, createMessage(unexpected)) {}
		
	private:
		static std::string createMessage(Token const& unexpected) {
			return fmt::format("Unexpected Token. Cannot start with token of type: {}.",
				tokenTypeToStr(unexpected.type));
		}
	};
	
	class TokenErrorMessage : public ParseError {
	public:
		TokenErrorMessage(Token const& extraInfo, std::string msg)
			: ParseError(extraInfo.sourcePos, std::move(msg)), extraInfo(extraInfo) {}
	private:
		Token extraInfo;
	};

};

