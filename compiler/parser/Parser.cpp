#include "Parser.h"
#include "Expr.h"
#include "spdlog\spdlog.h"

using enum Token::Type;

Stmt::Program Parser::program()
{	
	Stmt::Program program;
	while (!matchType(EOF_)) {
		tryToAddStmt(program);
	}
	return program;
}

void Parser::tryToAddStmt(Stmt::Program& program)
{
	try {
		program.push_back(stmt());
	}
	catch (ParseError& error) {
		spdlog::error(error.toString());
		synchronize();
	}
}

void Parser::synchronize()
{
	consumeWhile([](auto const& token) {
		return token.type != NEWLINE;
	});
	matchType(NEWLINE);
}
