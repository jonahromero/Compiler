#pragma once
#include <stdexcept>
#include <cassert>
#include "SourcePosition.h"
#include "spdlog/fmt/fmt.h"

#define COMPILER_NOT_SUPPORTED assert(false);
#define COMPILER_ASSERT(msg, x) assert((x) && (msg))
#ifdef NDEBUG
	#define COMPILER_DEBUG if constexpr(false)
#else
	#define COMPILER_DEBUG if constexpr(true) 
#endif

#ifdef __GNUC__
#define COMPILER_NOT_REACHABLE __builtin_unreachable();
#else
#define COMPILER_NOT_REACHABLE __assume(false);
#endif


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