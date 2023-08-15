
#include <unordered_map>
#include "Assembler.h"
#include "InstructionFormat.h"
#include "OperandDecoder.h"
#include <spdlog\spdlog.h>

namespace Asm {
	
	std::vector<Asm::Operand> Assembler::createOperands(Stmt::ArgList& argList) {
		if (argList.size() > 2) throw TooManyOperands();

		std::vector<Asm::Operand> retval;
		retval.reserve(argList.size());
		for (auto& arg : argList) {
			//retval.push_back(Asm::OperandDecoder(labelContext, pc).decodeExpr(arg));
		}
		return retval;
	}

	Bytes Assembler::assemble(Stmt::Instruction& instruct) {
		try {
			auto operands = createOperands(instruct.argList);
			auto instrucFormat = getInstructionFormat(instruct.opcode, operands);
			return FormatParser(instrucFormat.getFormat()).format(operands);
		}
		catch (TooManyOperands const& err) {
			spdlog::error("[Line: {}] No opcode takes more than 2 operands.", instruct.sourcePos.line);
		}
		catch (UnknownOpcode const& err) {
			spdlog::error("[Line: {}] Unknown Opcode: {}.", 
				instruct.sourcePos.line, instruct.opcode);
		}
		catch (InvalidOperands const& err) {
			spdlog::error("[Line: {}] Opcode: \"{}\" does not match specified operands.", 
				instruct.sourcePos.line, instruct.opcode);
		}
		return Bytes{};
	}
}
