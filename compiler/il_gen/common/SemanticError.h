#pragma once
#include "CompilerError.h"

class SemanticError : public CompilerError {
public:
    SemanticError(SourcePosition position, std::string msg) 
        : CompilerError(position), msg(std::move(msg)) {}

    std::string msgToString() {
        return msg;
    }
private:
    std::string msg;
};