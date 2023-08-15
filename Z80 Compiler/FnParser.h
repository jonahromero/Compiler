#pragma once
#include "ExprParser.h"


class FnParser
	: public ExprParser
{
public:
	FnParser(std::span<const Token> tokens)
		: ExprParser(tokens) {}

	Stmt::UniquePtr funcStmt();
protected:
	auto varDecl()->Stmt::VarDecl;
	auto argList()->Stmt::ArgList;
private:
	bool matchStmtTerminator();
	
//errors
protected:
	auto expectIdent() -> std::string_view;

	class InvalidFnStmt
		: public ParseError{
	public:
		InvalidFnStmt(Token token) 
			: ParseError(token.sourcePos), token(token) {}
		
		std::string msgToString() {
			return fmt::format("No statement starts with token of type: {}", tokenTypeToStr(token.type));
		}
	private:
		Token token;
	};
};

