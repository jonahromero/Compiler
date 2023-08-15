#pragma once
#include "ExprParser.h"
#include "spdlog\spdlog.h"


class BlockParser
	: public ExprParser
{
public:
	BlockParser(std::span<const Token> tokens)
		: ExprParser(tokens) {}

	Stmt::UniquePtr funcStmt();

protected:
	auto varDecl()->Stmt::VarDecl;
	auto decl()->Stmt::GenericDecl;
	auto stmtBlock()->std::vector<Stmt::UniquePtr>;

private:
	void expectStmtTerminator();
	bool matchStmtTerminator(); //currently not parsing \ as newline
	void synchronizeBlockStmt();

	auto opcode()->Stmt::UniquePtr;
	auto label()->Stmt::UniquePtr;
	auto ifStmt()->Stmt::UniquePtr;
	auto loopStmt()->Stmt::UniquePtr;
	auto returnStmt()->Stmt::UniquePtr;
	auto miscStmt()->Stmt::UniquePtr;
protected:
	auto letStmt(bool shouldExport)->Stmt::UniquePtr;

//errors
protected:

	template<typename Callable>
	auto tryExceptionalParsing(Callable callback, bool consumeNewline = true) {
		try {
			callback();
		}
		catch (ParseError& err) {
			spdlog::error(err.toString());
			synchronizeBlockStmt();
			if (consumeNewline)
				matchType(Token::Type::NEWLINE);
		}
	}
	template<typename LineParser, 
		typename LineType = std::invoke_result_t<LineParser>>
	auto parseGenericBlock(LineParser callback) -> std::vector<LineType> {
		using enum Token::Type;
		std::vector<LineType> retval;
		expectConsecutive(NEWLINE, INDENT);
		while (!matchType(DEDENT)) {
			tryExceptionalParsing([&]() {
				retval.push_back(callback());
			});
		}
		return retval;
	}

	template<typename Callable>
	auto safelyParseStmtBlockHeader(Callable callback) {
		tryExceptionalParsing(callback, false);
		return stmtBlock();
	}

	class InvalidFnStmt
		: public ParseError{
	public:
		InvalidFnStmt(Token token) 
			: ParseError(token.sourcePos), token(token) {}
		
		std::string msgToString() {
			return fmt::format("No statement starts with token of type: {}", tokenTypeToStr(token.type));
		}
	private:
		Token token;
	};
};

