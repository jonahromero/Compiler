#pragma once
#include <string>
#include "Operand.h"
#include "VariantUtil.h"
#include "StreamViewer.h"
#include "ExpectationErrors.h"

namespace Asm {

	class UnknownOpcode : std::exception {
	public:
		UnknownOpcode(std::string_view unknownOpcode)
			: unknownOpcode(unknownOpcode) {}
		std::string_view unknownOpcode;
	};

	class InvalidOperands : std::exception {
	public:
		InvalidOperands(std::vector<Operand> unknownOperands)
			: unknownOperands(std::move(unknownOperands)) {}
		std::vector<Operand> unknownOperands;
	};

	auto getInstructionFormat(
		std::string_view opcode, 
		std::vector<Operand> const& operands
	) -> class InstructionFormat const&;

	class InstructionFormat {
	public:

		class OperandRule {
		public:

			template<typename OperandType, typename...AcceptedValues>
			static OperandRule make(AcceptedValues...acceptedValues) {
				std::vector<Operand> validValues;
				(validValues.emplace_back(OperandType(acceptedValues)), ...);
				return OperandRule(variantIndex<Operand, OperandType>(), std::move(validValues));
			}
			bool allows(Operand const& operand) const;

		private:
			OperandRule(size_t operandIndex, std::vector<Operand> validValues)
				: operandIndex(operandIndex), validValues(validValues) {}
			size_t operandIndex;
			std::vector<Operand> validValues;
		};


		template<typename...Rules>
		InstructionFormat(std::string_view byteFormat, Rules...rule)
			: byteFormat(byteFormat) {
			(rules.push_back(rule), ...);
		}

		bool follows(std::vector<Operand> const& operands) const;
		auto getFormat() const->std::string_view;
	private:
		std::string_view byteFormat;
		std::vector<OperandRule> rules;
	};

	/*Format is modified hex numbers.
	* "<" will shift the previous Special Symbol and Bitwise Or the result into the last byte
	Special Symbols:
		O : offset, N : 8 bit number, NN : 16 bit number
		R : 8 bit Register, RR : 16 bit Register
		T : bit tested, P : (ph)flag, < : shift mapping and mask 
	*/
	class FormatParser {
	public:

		FormatParser(std::string_view byteFormat)
			: byteFormat(byteFormat) {}

		Bytes format(std::vector<Operand> const& operands);

	private:

		struct OperandSubstitutions {
			void operator()(Register const& operand);
			void operator()(OffsetRegister const& offsetReg);
			void operator()(Number const& number);
			void operator()(Flag const& flag);
			void operator()(Dereference const& deref);

			uint8_t r = 0, rr = 0, o = 0, t = 0, p = 0;
			uint16_t nn = 0; //theres no n, because we need to throw if we cant fit nn in n
		};

		class Filler
			: public StreamViewer<char> {
		public:
			Filler(std::string_view format)
				: StreamViewer<char>(format.data(), format.size()) {}
			Bytes getBytes(OperandSubstitutions const& subs);
		private:
			void addByte(uint8_t value);
			void shiftIntoLastByte(uint8_t value);
			void hex();
			virtual void reset() override;

			Bytes bytes;
			uint8_t leftShifts = 0;
		};

		auto calcOperandSubstitutions(std::vector<Operand> const& operands) -> OperandSubstitutions;
		std::string_view byteFormat;
	};
}
