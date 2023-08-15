#pragma once
#include <vector>
#include <span>
#include "Expr.h"
#include "Stmt.h"
#include "TokenViewer.h"

class Parser
	: public TokenViewer
{
public:
	using Program = std::vector<Stmt::UniquePtr>;

	Parser(std::span<const Token> tokens) 
		: TokenViewer(tokens.data(), tokens.size()) {}
	
	Program program();
	Stmt::UniquePtr stmt();
	Expr::UniquePtr expr();
private:	
	//error handling
	void tryToAddStmt(Program& program);
	void synchronize();
	//statement rules
	Stmt::ArgList argList();
	bool matchStmtTerminator();
	//expression rules
	Expr::UniquePtr logical();
	Expr::UniquePtr bitwise();
	Expr::UniquePtr comparison();
	Expr::UniquePtr equality();
	Expr::UniquePtr bitshift();
	Expr::UniquePtr term();
	Expr::UniquePtr factor();
	Expr::UniquePtr unary();
	Expr::UniquePtr primary();
	//helpers
	Token::Type previousType();

};

