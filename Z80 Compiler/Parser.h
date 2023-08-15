#pragma once
#include "StmtParser.h"

class Parser 
	: public StmtParser
{
public:
	Parser(std::span<const Token> tokens)
		: StmtParser(tokens) {}

	using Program = std::vector<Stmt::UniquePtr>;
	Program program();
private:	
	//error handling
	void tryToAddStmt(Program& program);
	void synchronize();
};
