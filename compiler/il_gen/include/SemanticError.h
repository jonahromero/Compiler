#pragma once
#include "CompilerError.h"

class SemanticError : public CompilerError {
public:
    SemanticError(SourcePosition position, const char* msg) 
        : CompilerError(position) {}

    std::string msgToString() {
        return msg;
    }
private:
    const char* msg;
};