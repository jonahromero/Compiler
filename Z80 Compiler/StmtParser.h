#pragma once
#include "BlockParser.h"

class StmtParser 
	: public BlockParser
{
public:

	StmtParser(std::span<const Token> tokens)
		: BlockParser(tokens) {}

	Stmt::UniquePtr stmt();
private:
	auto funcParams()->std::vector<Stmt::VarDecl>;
	auto templateDecl()->Stmt::TemplateDecl;
	auto binBody()->std::vector<Stmt::VarDecl>;

	auto importStmt(bool shouldExport)->Stmt::UniquePtr;
	auto globalDecl(bool shouldExport)->Stmt::UniquePtr;
	auto bin(bool shouldExport)->Stmt::UniquePtr;
	auto fn(bool shouldExport)->Stmt::UniquePtr;

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
	class InvalidBinDecl : public ParseError {
	public:
		InvalidBinDecl(Token const& start)
			: ParseError(start.sourcePos), start(start) {}

		std::string msgToString() {
			return fmt::format("Invalid Bin Decleration starting with: '{}'", tokenTypeToStr(start.type));
		}
	private:
		Token start;
	};
};

