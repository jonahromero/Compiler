#pragma once
#include <stdexcept>
#include "Token.h"
#include "spdlog/fmt/fmt.h"
#define NOT_SUPPORTED assert(false);
#define NOT_REACHABLE assert(false);


class CompilerError : std::exception {
public:

	CompilerError(Token::SourcePosition sourcePos)
		: sourcePos(sourcePos) {}

	std::string toString() {
		return fmt::format("[Line: {}] ", sourcePos.line) + msgToString();
	}
private:
	Token::SourcePosition sourcePos;
	virtual std::string msgToString() = 0;
};