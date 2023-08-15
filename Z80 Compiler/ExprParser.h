#pragma once
#include <vector>
#include <span>
#include <array>
#include "TokenViewer.h"
#include "Expr.h"
#include "Stmt.h"
#include "ParseError.h"

class ExprParser
	: public TokenViewer
{
public:
	ExprParser(std::span<const Token> tokens)
		: TokenViewer(tokens.data(), tokens.size()) {}

	Expr::UniquePtr expr();
private:
	Expr::UniquePtr logical();
	Expr::UniquePtr bitwise();
	Expr::UniquePtr comparison();
	Expr::UniquePtr equality();
	Expr::UniquePtr bitshift();
	Expr::UniquePtr term();
	Expr::UniquePtr factor();
	Expr::UniquePtr unary();
	Expr::UniquePtr primary();

protected:
	Token::Type previousType() const;
	Token::SourcePosition previousSourcePos() const;

	void expect(Token::Type expected);
	auto expectIdent()->std::string_view;
	auto argList()-> Stmt::ArgList;

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
			: ParseError(found.sourcePos), expectedType(expectedType), found(found) {}
		
		std::string msgToString() override {
			if (found.type == Token::Type::EOF_) {
				return fmt::format("Expected a token of type: {}, but reached the end of the file.",
					tokenTypeToStr(expectedType));
			}
			else {
				return fmt::format("Expected a token of type: {}, but found a {}.",
					tokenTypeToStr(expectedType), tokenTypeToStr(found.type));
			}
		}
	private:
		Token::Type expectedType;
		Token found;
	};

	class UnexpectedToken : public ParseError {
	public:
		UnexpectedToken(Token const& unexpected)
			: ParseError(unexpected.sourcePos), unexpected(unexpected) {}
		
		std::string msgToString() override {
			return fmt::format("Unexpected Token. Cannot start with token of type: {}.",
				tokenTypeToStr(unexpected.type));
		}
	private:
		Token unexpected;
	};
	
	class TokenErrorMessage : public ParseError {
	public:
		TokenErrorMessage(Token const& extraInfo, std::string_view msg)
			: ParseError(extraInfo.sourcePos), extraInfo(extraInfo), msg(msg) {}

		std::string msgToString() override {
			return std::string(msg);
		}
	private:
		std::string_view msg;
		Token extraInfo;
	};

};

