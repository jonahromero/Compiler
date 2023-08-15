#pragma once
#include <cstdint>
#include "Stmt.h"
#include "Parser.h"
#include "Assembler.h"

/*Basically just an assembler wrapper, but will eventually support lang features*/

class Compiler  
{
public:
	Compiler(std::string_view input);
	Asm::Bytes compile();
private:
	friend class CompilerVisitor;

	void addLabel(std::string_view label);
	auto assembleInstruction(Stmt::Instruction& instruction) -> Asm::Bytes;
	auto createOperands(Stmt::ArgList& argList) -> std::vector<Asm::Operand>;

	Parser::Program program;
	Asm::LabelContext labelContext;
	uint16_t pc = 0;
};

