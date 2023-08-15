#include "Compiler.h"
#include "Parser.h"
#include "Lexer.h"
#include "spdlog/spdlog.h"

Compiler::Compiler(std::string_view input)
{
	auto tokens = Lexer(input).generateTokens();
    program = Parser(tokens).program();
}

Asm::Bytes Compiler::compile()
{
	Asm::Bytes bytes;
	for (auto& stmt : program) {
		auto newBytes = visitPtr(stmt);
		bytes.insert(bytes.end(), newBytes.begin(), newBytes.end());
		//pc is not just size of bytes, since it can start somewhere besides 0. No assignment
		pc += static_cast<uint16_t>(newBytes.size());
	}
	return bytes;
}

void Compiler::runIdentifierPass()
{
	for (auto& stmt : program) {
		//auto newBytes = visitPtr(stmt);
		//bytes.insert(bytes.end(), newBytes.begin(), newBytes.end());
		//pc is not just size of bytes, since it can start somewhere besides 0. No assignment
		//pc += static_cast<uint16_t>(newBytes.size());
	}
}


void Compiler::visit(Stmt::Label& label)
{
	labelContext[label.label] = pc;
	returnValue(Asm::Bytes{});
}

void Compiler::visit(Stmt::Instruction& stmt)
{
	if (!isIdentifierPass) {
		returnValue(Asm::Assembler(labelContext, pc).assemble(stmt));
	}
	else {
		returnValue(Asm::Bytes{});
	}
}
