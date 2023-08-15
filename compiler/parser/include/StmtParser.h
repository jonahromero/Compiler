#pragma once
#include "BlockParser.h"

class StmtParser 
	: public BlockParser
{
public:

	StmtParser(std::span<const Token> tokens, ParserContext& context)
		: BlockParser(tokens, context), context(context) {}

	Stmt::UniquePtr stmt();
private:
	ParserContext& context;

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
			: ParseError(start.sourcePos, createMessage(start)) {}
		
	private:
		static std::string createMessage(Token const& start) {
			return fmt::format("Statement cannot start with a token of type: {}", tokenTypeToStr(start.type));
		}
	};
	class InvalidBinDecl : public ParseError {
	public:
		InvalidBinDecl(Token const& start)
			: ParseError(start.sourcePos, createMessage(start)) {}

	private:
		static std::string createMessage(Token const& start) {
			return fmt::format("Invalid Bin Decleration starting with: '{}'", tokenTypeToStr(start.type));
		}
	};
};

