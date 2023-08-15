#pragma once
#include "FnParser.h"

class StmtParser 
	: public FnParser
{
public:

	StmtParser(std::span<const Token> tokens)
		: FnParser(tokens) {}

	Stmt::UniquePtr stmt();
private:
	auto funcParams()->std::vector<Stmt::VarDecl>;
	auto templateDecl()->Stmt::TemplateDecl;
	auto binBody()->std::vector<Stmt::VarDecl>;
	auto fnBody()->std::vector<Stmt::UniquePtr>;

	class InvalidModuleStmt : public ParseError {
	public:
		InvalidModuleStmt(Token const& start)
			: ParseError(start.sourcePos), start(start) {}
		
		std::string msgToString() {
			return fmt::format("Statement cannot start with a token of type: {}", tokenTypeToStr(start.type));
		}
	private:
		Token start;
	};
};

