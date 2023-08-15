#pragma once
#include <stdexcept>
#include "SourcePosition.h"
#include "spdlog/fmt/fmt.h"

#define NOT_SUPPORTED assert(false);
#define NOT_REACHABLE assert(false);

class CompilerError : std::exception 
{
public:
	CompilerError(SourcePosition sourcePos)
		: sourcePos(sourcePos) {}

	std::string toString() {
		return fmt::format("[Line: {}] ", sourcePos.line) + msgToString();
	}
private:
	SourcePosition sourcePos;
	virtual std::string msgToString() = 0;
};