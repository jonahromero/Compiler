#pragma once
#include <array>
#include "Stmt.h"
#include "Operand.h"

namespace Asm {

	class Assembler {
	public:
		Assembler(LabelContext const& labelContext, size_t pc)
			: labelContext(labelContext), pc(pc) {}
		Bytes assemble(Stmt::Instruction& instruct);

	private:
		class TooManyOperands {};
		auto createOperands(Stmt::ArgList& argList) -> std::vector<Asm::Operand>;

		LabelContext const& labelContext;
		size_t pc;
	};
}
