#include "StmtParser.h"

using enum Token::Type;

Stmt::UniquePtr StmtParser::stmt()
{
	bool isExporting = matchType(EXPORT);

	if (matchType(IMPORT)) {
		auto sourcePos = previousSourcePos();
		if (matchType(STRING))
			return Stmt::makeStmt<Stmt::Import>(sourcePos, peekPrevious().lexeme);
		else
			throw TokenErrorMessage(peek(), "Import statement expects a string");
		expect(NEWLINE);
	}
	else if (matchType(FN)) {
		Stmt::Function func;
		func.isExported = isExporting;
		auto sourcePos = previousSourcePos();
		if (matchType(LESS)) {
			func.templateInfo = templateDecl();
		}
		func.name = expectIdent();
		expect(LEFT_PARENTH);
		func.params = funcParams();
		expectConsecutive(COLON, NEWLINE, INDENT);
		func.body = fnBody();
		return Stmt::makeStmt<Stmt::Function>(sourcePos, std::move(func));

	}
	else if (matchType(BIN)) {
		Stmt::Bin bin;
		auto sourcePos = previousSourcePos();
		bin.isExported = isExporting;
		if (matchType(LESS)) {
			bin.templateInfo = templateDecl();
		}
		if (matchType(IDENT)) {
			bin.name = peekPrevious().lexeme;
			expectConsecutive(COLON, NEWLINE, INDENT);
			bin.body = binBody();
			return Stmt::makeStmt<Stmt::Bin>(sourcePos, std::move(bin));
		}
		else {
			throw TokenErrorMessage(peek(), "Bin keyword must be followed by identifier.");
		}
	}
	//special condition, because let is not required if exporting
	else if (matchType(LET) || (matchType(IDENT) && isExporting)) {

	}
	else if (matchType(TYPE)) {

	}
	else {
		throw InvalidModuleStmt(peek());
	}
}

//statements

auto StmtParser::binBody() -> std::vector<Stmt::VarDecl>
{
	std::vector<Stmt::VarDecl> retval;
	while (!matchType(DEDENT)) {
		retval.push_back(varDecl());
		expect(NEWLINE);
	}
	return retval;
}

auto StmtParser::funcParams() -> std::vector<Stmt::VarDecl>
{
	std::vector<Stmt::VarDecl> retval;
	while (!matchType(RIGHT_PARENTH)) {
		retval.push_back(varDecl());
	}
	return retval;
}

auto StmtParser::templateDecl() -> Stmt::TemplateDecl
{
	Stmt::TemplateDecl retval;
	if (matchType(GREATER))
		throw TokenErrorMessage(peek(), "Empty Template Declerations not allowed");;
	do {
		auto name = expectIdent();
		expect(COLON);
		bool isMut = matchType(MUT);
		if (matchType(IDENT)) {
			retval.addDecl(Stmt::VarDecl(name, Stmt::Type(peekPrevious().lexeme, isMut)));
		}
		else if (matchType(TYPE)) {
			if (isMut) throw TokenErrorMessage(peek(), "mut is not a valid specifier for keyword \'Type\'");
			retval.addDecl(Stmt::TypeDecl(name));
		}
		else {
			throw TokenErrorMessage(peek(), "Expected Type After variable decleration");
		}
	} while (matchType(COMMA));
	expect(GREATER);
	return retval;
}

auto StmtParser::fnBody() -> std::vector<Stmt::UniquePtr>
{
	std::vector<Stmt::UniquePtr> retval;
	while (!matchType(DEDENT)) {
		retval.push_back(funcStmt());
	}
	return retval;
}




