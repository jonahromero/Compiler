#include "Compiler.h"
#include "Parser.h"
#include "Lexer.h"
#include "OperandDecoder.h"
#include "spdlog/spdlog.h"

class CompilerVisitor 
	: public Stmt::VisitorReturner<Asm::Bytes> {
public:
	CompilerVisitor(Compiler& compiler) 
		: compiler(compiler) {}

	virtual void visit(Stmt::Label& label) {
		compiler.addLabel(label.label);
		returnValue(Asm::Bytes{});
	}
	virtual void visit(Stmt::Instruction& stmt) {
		returnValue(compiler.assembleInstruction(stmt));
	}
private:
	Compiler& compiler;
};

Compiler::Compiler(std::string_view input)
{
    auto tokens = Lexer(input).generateTokens();
    program = Parser(tokens).program();
}

Asm::Bytes Compiler::compile()
{
	Asm::Bytes bytes;
	for (auto& stmt : program) {
		auto newBytes = stmt->accept(CompilerVisitor(*this));
		bytes.insert(bytes.end(), newBytes.begin(), newBytes.end());
		//pc is not just size of bytes, since it can start somewhere besides 0. No assignment
		pc += static_cast<uint16_t>(newBytes.size());
	}
	return bytes;
}

void Compiler::addLabel(std::string_view label)
{
	labelContext[label] = pc;
}

std::vector<Asm::Operand> Compiler::createOperands(Stmt::ArgList& argList) {
	if (argList.size() > 2) throw "? I suppose no more than 2 ops";
	std::vector<Asm::Operand> retval; retval.reserve(argList.size());
	for (auto& arg : argList) {
		Asm::Operand newOperand = Asm::OperandDecoder(labelContext, pc).decodeExpr(arg);
		retval.push_back(newOperand);
	}
	return retval;
}

Asm::Bytes Compiler::assembleInstruction(Stmt::Instruction& instruction)
{
	auto operands = createOperands(instruction.argList);
    return Asm::assemble(instruction.opcode, operands);
}
