#pragma once
#include <unordered_map>
#include <variant>
#include <string>

namespace Asm {

	struct Number {
		Number(uint16_t val) : val(val) {}
		auto operator<=>(const Number&) const = default;
		uint16_t val;
	};
	struct Flag {
		Flag(std::string_view str) : str(str) {}
		auto operator<=>(const Flag&) const = default;
		std::string_view str;
	};
	struct Register {
		Register(std::string_view str) : str(str) {}
		auto operator<=>(const Register&) const = default;
		std::string_view str;
	};
	struct OffsetRegister : public Register {
		OffsetRegister(std::string_view reg, uint16_t offset) 
			: Register{reg}, offset(offset) {}
		auto operator<=>(const OffsetRegister& other) const {
			return this->str <=> other.str;
		}
		uint8_t offset;
	};

	using Dereferenceable = std::variant<Register, OffsetRegister, Number>;
	struct Dereference {
		Dereference(Dereferenceable address) : address(address) {}
		auto operator<=>(const Dereference&) const = default;
		Dereferenceable address;
	};

	using Operand = std::variant<Register, OffsetRegister, Flag, Number, Dereference>;
	using Bytes = std::vector<uint8_t>;
	using LabelContext = std::unordered_map<std::string_view, uint16_t>;

	struct Instruction {
		std::string_view opcode;
		std::vector<Asm::Operand> operands;
	};
	using AsmBlock = std::vector<Instruction>;

}