#pragma once
#include "StmtParser.h"

class Parser 
	: public StmtParser
{
public:
	Parser(std::span<const Token> tokens)
		: StmtParser(tokens, context) {}

	Stmt::Program program();
private:
	ParserContext context;
	//error handling
	void tryToAddStmt(Stmt::Program& program);
	void synchronize();
};
