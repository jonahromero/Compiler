#pragma once
#include <array>
#include "Stmt.h"
#include "Operand.h"

namespace Asm {
	Bytes assemble(std::string_view opcode, std::vector<Operand> const& operands);
}

