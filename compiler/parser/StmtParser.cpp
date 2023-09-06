#include "StmtParser.h"

using enum Token::Type;

Stmt::UniquePtr StmtParser::stmt()
{
	bool isExporting = matchType(EXPORT);
	auto token = advance();
	switch (token.type) {
	case IMPORT: return importStmt(isExporting);
	case LET: return globalDecl(isExporting);
	case FN: return fn(isExporting);
	case BIN: return bin(isExporting);
	default:;
	}
	//special condition, because let is not required if exporting
	if (isExporting) {
		return globalDecl(isExporting);
	}
	else {
		throw InvalidModuleStmt(peek());
	}
}

//statements

auto StmtParser::binBody() -> std::vector<Stmt::VarDecl>
{
	return parseGenericBlock([&]() { 
		auto current = peek();
		auto tempDecl = decl();
		if (std::holds_alternative<Stmt::TypeDecl>(tempDecl)) {
			throw InvalidBinDecl(current);
		}
		expect(NEWLINE);
		return std::move(std::get<Stmt::VarDecl>(tempDecl));
	});
}

auto StmtParser::importStmt(bool shouldExport) -> Stmt::UniquePtr
{
	if (shouldExport) TokenErrorMessage(peek(), "Import Statement cannot be exported.");
	auto sourcePos = previousSourcePos();
	if (matchType(STRING)) {
		auto file = peekPrevious().lexeme;
		expect(NEWLINE);
		return Stmt::makeStmt<Stmt::Import>(sourcePos, file);
	}
	else {
		throw TokenErrorMessage(peek(), "Import statement expects a string.");
	}
}

auto StmtParser::globalDecl(bool shouldExport) -> Stmt::UniquePtr
{
	return letStmt(shouldExport);
}

auto StmtParser::fn(bool shouldExport) -> Stmt::UniquePtr
{
	Stmt::Function func;
	auto sourcePos = previousSourcePos();
	func.isExported = shouldExport;
	func.body = safelyParseStmtBlockHeader([&]() {
		if (matchType(LESS)) {
			func.templateInfo = templateDecl();
		}
		func.name = expectIdent();
		if(func.isTemplate()) {
			context.addTemplate(func.name);
		}
		expect(LEFT_PARENTH);
		func.params = funcParams();
		expect(ARROW);
		if (!matchType(COLON)) {
			func.retType = expr();
			expectConsecutive(COLON);
		}
	});
	return Stmt::makeStmt<Stmt::Function>(sourcePos, std::move(func));
}

auto StmtParser::bin(bool shouldExport) -> Stmt::UniquePtr
{
	Stmt::Bin bin;
	auto sourcePos = previousSourcePos();
	bin.isExported = shouldExport;
	tryExceptionalParsing([&]() {
		if (matchType(LESS)) {
			bin.templateInfo = templateDecl();
		}
		expect(IDENT);
		bin.name = peekPrevious().lexeme;
		if (bin.isTemplate()) {
			context.addTemplate(bin.name);
		}
		expect(COLON);
	}, false);
	bin.body = binBody();
	return Stmt::makeStmt<Stmt::Bin>(sourcePos, std::move(bin));
}

auto StmtParser::funcParams() -> std::vector<Stmt::VarDecl>
{
	std::vector<Stmt::VarDecl> retval;
	if (!matchType(RIGHT_PARENTH)) {
		do {
			retval.push_back(varDecl());
		} while (matchType(COMMA));
		expect(RIGHT_PARENTH);
	}
	return retval;
}

auto StmtParser::templateDecl() -> Stmt::TemplateDecl
{
	Stmt::TemplateDecl retval;
	if (matchType(GREATER))
		throw TokenErrorMessage(peek(), "Empty Template Declerations not allowed");
	auto scopedChange = context.turnOnTemplateMode();
	do {
		retval.params.push_back(decl());
	} while (matchType(COMMA));
	expect(GREATER);
	return retval;
}





