#include "BlockParser.h"

using enum Token::Type;

Stmt::UniquePtr BlockParser::funcStmt()
{
	if (matchType(OPCODE)) return opcode();
	else if (matchType(IF)) return ifStmt();
	else if (matchType(COUNT)) return loopStmt();
	else if (matchType(RETURN)) return returnStmt();
	else if (matchType(LET)) return letStmt(false);
	//case IDENT: no defined path
	else return miscStmt();
	__assume(false); //unreachable code
}


auto BlockParser::opcode()->Stmt::UniquePtr
{
	SourcePosition sourcePos = previousSourcePos();
	auto opcode = peekPrevious().lexeme;
	auto args = argList();
	expect(NEWLINE);
	return Stmt::makeStmt<Stmt::Instruction>(sourcePos, opcode, std::move(args));
}

auto BlockParser::label()->Stmt::UniquePtr
{
	SourcePosition sourcePos = previousSourcePos();
	auto ident = peekPrevious().lexeme;
	expect(COLON); matchType(NEWLINE);
	return Stmt::makeStmt<Stmt::Label>(sourcePos, ident);
}

auto BlockParser::ifStmt()->Stmt::UniquePtr
{
	SourcePosition sourcePos = previousSourcePos();
	Stmt::If ifStmt;
	ifStmt.ifBranch.body = safelyParseStmtBlockHeader([&]() {
		ifStmt.ifBranch.expr = expr();
		expect(COLON);
	});
	while (matchType(ELSE)) {
		if (matchType(IF)) {
			Stmt::Conditional conditional;
			conditional.body = safelyParseStmtBlockHeader([&]() {
				conditional.expr = expr();
				expect(COLON);
			});
			ifStmt.elseIfBranch.push_back(std::move(conditional));
		}
		else {
			ifStmt.elseBranch = safelyParseStmtBlockHeader([&]() {
				expect(COLON);
			});
			break;
		}
	}
	return Stmt::makeStmt<Stmt::If>(sourcePos, std::move(ifStmt));
}

auto BlockParser::loopStmt()->Stmt::UniquePtr
{
	SourcePosition sourcePos = previousSourcePos();
	Stmt::CountLoop loop;
	loop.body = safelyParseStmtBlockHeader([&]() {
		expect(WITH);
		loop.counter = expectIdent();
		expect(FROM);
		loop.initializer = expr();
		expectConsecutive(COLON);
	});
	return Stmt::makeStmt<Stmt::CountLoop>(sourcePos, std::move(loop));
}

auto BlockParser::returnStmt()->Stmt::UniquePtr
{
	SourcePosition sourcePos = previousSourcePos();
	Stmt::Return returnStmt;
	returnStmt.expr = expr();
	expect(NEWLINE);
	return Stmt::makeStmt<Stmt::Return>(sourcePos, std::move(returnStmt));
}

auto BlockParser::miscStmt() -> Stmt::UniquePtr
{
	if (peek().type == IDENT && peekNext().type == COLON) {
		advance(); //label expects to be next spot
		return label();
	}
	else {
		SourcePosition sourcePos = previousSourcePos();
		auto lhs = expr();
		if (matchType(EQUAL)) {
			auto rhs = expr();
			expect(NEWLINE);
			return Stmt::makeStmt<Stmt::Assign>(sourcePos, std::move(lhs), std::move(rhs));
		}
		else {
			expect(NEWLINE);
			return Stmt::makeStmt<Stmt::ExprStmt>(sourcePos, std::move(lhs));
		}
	}
}

auto BlockParser::letStmt(bool shouldExport)->Stmt::UniquePtr
{
	SourcePosition sourcePos = previousSourcePos();
	Stmt::VarDef var(decl(), shouldExport);
	if (matchType(EQUAL)) {
		var.initializer = expr();
	}
	expect(NEWLINE);
	return Stmt::makeStmt<Stmt::VarDef>(sourcePos, std::move(var));
}

//stmt helpers

auto BlockParser::stmtBlock() -> std::vector<Stmt::UniquePtr>
{
	return parseGenericBlock([&]() { return funcStmt(); });
}

auto BlockParser::varDecl() -> Stmt::VarDecl
{
	auto name = expectIdent();
	expect(COLON);
	return Stmt::VarDecl(name, expr());
}

auto BlockParser::decl() -> Stmt::GenericDecl
{
	//returns vardecl from identifier to type
	auto name = expectIdent();
	expect(COLON);
	if (matchType(TYPE)) {
		return Stmt::TypeDecl(name);
	}
	else {
		return Stmt::VarDecl(name, expr());
	}
}

//Errror Handling and yea fuck this

void BlockParser::expectStmtTerminator()
{
	if (matchType(BACKSLASH)) return;
	expect(NEWLINE);
}

bool BlockParser::matchStmtTerminator()
{
	return matchType(NEWLINE) || matchType(BACKSLASH);
}

void BlockParser::synchronizeBlockStmt()
{
	consumeWhile([](auto token) {
		return token.type != NEWLINE && token.type != DEDENT;
	});
}

