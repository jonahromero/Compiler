#pragma once
#include "StmtParser.h"

class Parser 
	: public StmtParser
{
public:
	Parser(std::span<const Token> tokens)
		: StmtParser(tokens) {}

	Stmt::Program program();
private:	
	//error handling
	void tryToAddStmt(Stmt::Program& program);
	void synchronize();
};
