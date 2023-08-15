#pragma once
#include <unordered_map>
#include <stdexcept>
#include "Token.h"
#include "spdlog/fmt/fmt.h"

class ParseError : std::exception {
public:
	ParseError(Token::SourcePosition sourcePos) 
		: sourcePos(sourcePos) {}
	
	std::string toString() {
		return fmt::format("[Line: {}] ", sourcePos.line) + msgToString();
	}
private:
	Token::SourcePosition sourcePos;
	virtual std::string msgToString() = 0;
};