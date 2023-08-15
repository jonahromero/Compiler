#include "FnParser.h"


using enum Token::Type;

Stmt::UniquePtr FnParser::funcStmt()
{
	if (matchType(OPCODE)) {
		Token::SourcePosition sourcePos = previousSourcePos();
		auto opcode = peekPrevious().lexeme;
		auto args = argList();
		return Stmt::makeStmt<Stmt::Instruction>(sourcePos, opcode, std::move(args));
	}
	else if (matchType(IDENT)) {
		Token::SourcePosition sourcePos = previousSourcePos();
		auto ident = peekPrevious().lexeme;
		expect(COLON);
		return Stmt::makeStmt<Stmt::Label>(sourcePos, ident);
	}
	else if (matchType(NEWLINE)) {
		throw "Unexpected Newline character";
		return Stmt::makeStmt<Stmt::NullStmt>(previousSourcePos());
	}
	else {
		throw InvalidFnStmt(peek());
	}
}


Stmt::ArgList FnParser::argList()
{
	Stmt::ArgList retval;
	if (!matchStmtTerminator()) {
		retval.push_back(expr());
		while (!matchStmtTerminator()) {
			expect(COMMA);
			retval.push_back(expr());
		}
	}
	return retval;
}

auto FnParser::varDecl() -> Stmt::VarDecl
{
	auto name = expectIdent();
	expect(COLON);
	bool isMut = matchType(MUT);
	return Stmt::VarDecl(name, Stmt::Type(expectIdent(), isMut));
}

bool FnParser::matchStmtTerminator()
{
	return matchType(NEWLINE) || matchType(BACKSLASH);
}

auto FnParser::expectIdent() -> std::string_view
{
	expect(IDENT);
	return peekPrevious().lexeme;
}
