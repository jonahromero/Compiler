#include "InstructionFormat.h"
#include "spdlog\spdlog.h"

namespace Asm {

	auto findValidRule(
		std::vector<InstructionFormat> const& formats, 
		std::vector<Operand> const& operands
	) -> std::string_view 
	{
		for (auto& format : formats) {
			if (format.follows(operands)) {
				return format.getFormat();
			}
		}
		throw InvalidOperands(operands);
	}

	auto getInstructionFormat(
		std::string_view opcode, 
		std::vector<Operand> const& operands
	) -> InstructionFormat const&
	{
		using IFormat = InstructionFormat;
		using Rule = IFormat::OperandRule;

		static auto reg8 = Rule::make<Register>("a", "b", "c", "d", "e", "h", "l");
		static auto reg16 = Rule::make<Register>("bc", "de", "hl", "sp");
		static auto shadowRegAF = Rule::make<Register>(R"(af')");
		static auto extendIX = Rule::make<Register>("ixh", "ixl");
		static auto extendIY = Rule::make<Register>("iyh", "iyl");


		static auto regA = Rule::make<Register>("a");
		static auto regF = Rule::make<Register>("f");
		static auto regHL = Rule::make<Register>("hl");
		static auto regSP = Rule::make<Register>("sp");
		static auto regDE = Rule::make<Register>("de");
		static auto regIX = Rule::make<Register>("ix");
		static auto regIY = Rule::make<Register>("iy");

		static auto number = Rule::make<Number>();
		static auto flag = Rule::make<Flag>();

		static auto derefIX = Rule::make<Dereference>(Dereference{ OffsetRegister{{"ix"}, 0} });
		static auto derefIY = Rule::make<Dereference>(Dereference{ OffsetRegister{{"iy"}, 0} });
		static auto derefC  = Rule::make<Dereference>(Dereference{ Register("c") });
		static auto derefHL = Rule::make<Dereference>(Dereference{ Register("hl") });
		static auto derefSP = Rule::make<Dereference>(Dereference{ Register("sp") });

		using OpcodeFormats = std::unordered_map<std::string_view, std::vector<InstructionFormat>>;

		//WE NEED TO FIX OFFSET REGISTERS, SHADOW REGISTERS, AND NUMBERS IN "IM"
		static OpcodeFormats opcodeFormats = {
			{"adc",{
				IFormat("8E",	 regA, derefHL),
				IFormat("DD8EO", regA, derefIX),
				IFormat("FD8EO", regA, derefIY),
				IFormat("88R",	 regA, reg8),
				IFormat("DD88R", regA, extendIX),
				IFormat("FD88R", regA, extendIY),
				IFormat("CEN",	 regA, number),
				IFormat("ED4A<<<<RR", derefHL, reg16)
			}},
			{"add",{
				IFormat("86",	 regA, derefHL),
				IFormat("DD86O", regA, derefIX),
				IFormat("FD86O", regA, derefIY),
				IFormat("80R",	 regA, reg8),
				IFormat("DD80R", regA, extendIX),
				IFormat("FD80R", regA, extendIY),
				IFormat("C6N",	 regA, number),

				IFormat("09<<<<RR", regHL, reg16),
				IFormat("DD09<<<<RR", regIX, reg16),
				IFormat("FD09<<<<RR", regIY, reg16)
			}},
			{"and",{
				IFormat("A6",	 derefHL),
				IFormat("DDA6O", derefIX),
				IFormat("FDA6O", derefIY),
				IFormat("A0R",	 regA, reg8),
				IFormat("DDA0R", regA, extendIX),
				IFormat("FDA0R", regA, extendIY),
				IFormat("E6N",	 regA, number)
			}},
			{"bit",{
				IFormat("CB46<<<T",		derefHL),
				IFormat("DDCBO46<<<T",	derefIX),
				IFormat("FDCBO46<<<T",	derefIY),
				IFormat("CB60R<<<T",	number, reg8)
			}},
			{"call",{
				IFormat("CDNN",	 number),
				IFormat("C4<<<PNN", flag, number)
			}},
			{"ccf", { IFormat("3F")	}},
			{"cp",{
				IFormat("BE",	 derefHL),
				IFormat("DDBEO", derefIX),
				IFormat("FDBEO", derefIY),
				IFormat("B8R",	 reg8),
				IFormat("FEN",	 number)
			}},
			{"cpd",	 { IFormat("EDA9") }},
			{"cpdr", { IFormat("EDB9") }},
			{"cpi",  { IFormat("EDA1") }},
			{"cpir", { IFormat("EDB1") }},
			{"cpl",  { IFormat("2F") }},
			{"daa",  { IFormat("27") }},

			{"dec", {
				IFormat("35", derefHL),
				IFormat("DD350", derefIX),
				IFormat("FD350", derefIY),
				IFormat("05<<<R", reg8),
				IFormat("DDA0<<<R", regA, extendIX),
				IFormat("FDA0<<<R", regA, extendIY),
				IFormat("03<<<<RR", reg16)
			}},

			{"di",  { IFormat("F3") }},
			{"djnz",  {
				IFormat("10N", number)
			}},
			{"ei",  { IFormat("FB") }},

			{"ex", {
				IFormat("E3", derefSP, regHL),
				IFormat("DDE3", derefSP, regIX),
				IFormat("FDE3", derefSP, regIY),
				//IFormat("08", shadowRegisterAF,),
				IFormat("EB", regDE, regHL)
			}},
			{"exx",  { IFormat("D9") }},
			{"halt",  { IFormat("76") }},
			//IM 0
			//IM 1
			//IM 2
			{"in", {
				IFormat("ED40<<<R", reg8, derefC),
				IFormat("ED70", regF, derefC),
				IFormat("DBN", regA, number)
			}},
			{"inc", {
				IFormat("34", derefHL),
				IFormat("DD340", derefIX),
				IFormat("FD340", derefIY),
				IFormat("03<<<<RR", regHL),
				IFormat("DD03<<<<RRO", regIX),
				IFormat("FD03<<<<RRO", regIY),
			}},

			{"nop",  { IFormat("00") }},
		};

		auto opcodeInstructionFormats = opcodeFormats.find(opcode);
		if (opcodeInstructionFormats == opcodeFormats.end()) {
			throw UnknownOpcode(opcode);
		}
		return findValidRule(opcodeInstructionFormats->second, operands);
	}
}