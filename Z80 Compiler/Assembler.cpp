
#include <unordered_map>
#include "Assembler.h"
#include "InstructionFormat.h"

namespace Asm {
	Bytes assemble(std::string_view opcode, std::vector<Operand> const& operands) {
		auto instrucFormat = getInstructionFormat(opcode, operands);
		return FormatParser(instrucFormat.getFormat()).format(operands);
	}
}
