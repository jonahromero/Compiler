#include "InstructionFormat.h"
#include "OperandFormatBits.h"

namespace Asm {

	bool InstructionFormat::OperandRule::allows(Operand const& operand) const {
		//no validValues means, everythings valid
		if (validValues.size() == 0) return true;
		for (auto const& validValue : validValues) {
			if (operand == validValue) return true;
		}
		return false;
	}

	bool InstructionFormat::follows(std::vector<Operand> const& operands) const
	{
		if (operands.size() != rules.size()) return false;
		for (int i = 0; i < operands.size(); ++i) {
			if (!rules[i].allows(operands[i])) return false;
		}
		return true;
	}

	auto InstructionFormat::getFormat() const -> std::string_view {
		return byteFormat;
	}

	Bytes FormatParser::format(std::vector<Operand> const& operands)
	{
		auto subs = calcOperandSubstitutions(operands);
		return Filler(byteFormat).getBytes(subs);
	}

	auto FormatParser::calcOperandSubstitutions(std::vector<Operand> const& operands)
		-> FormatParser::OperandSubstitutions
	{
		OperandSubstitutions operandSubs;
		for (auto& operand : operands) {
			std::visit(operandSubs, operand);
		}
		return operandSubs;
	}

	void FormatParser::OperandSubstitutions::operator()(Register const& reg)
	{
		auto bits = reg16ToBits(reg.str);
		if (bits.has_value()) {
			rr = bits.value();
		}
		else {
			auto bits = reg8ToBits(reg.str);
			r = bits.value();
		}
	}
	void FormatParser::OperandSubstitutions::operator()(OffsetRegister const& offsetReg)
	{
		o = offsetReg.offset;
	}
	void FormatParser::OperandSubstitutions::operator()(Number const& number)
	{
		nn = number.val;
		t = static_cast<uint8_t>(nn);
	}
	void FormatParser::OperandSubstitutions::operator()(Flag const& flag)
	{
		p = flagToBits(flag.str).value();
	}
	void FormatParser::OperandSubstitutions::operator()(Dereference const& deref)
	{
		std::visit([&](auto&& address) {
			this->operator()(address);
			}, deref.address);
	}

	Bytes FormatParser::Filler::getBytes(OperandSubstitutions const& subs)
	{
		reset();
		while (!atEnd()) {
			switch (advance()) {
			case 'R':
				match('R') ? shiftIntoLastByte(subs.rr) : shiftIntoLastByte(subs.r);
				break;
			case 'N':
				match('N') ? (
					addByte(static_cast<uint8_t>(subs.nn)),
					addByte(static_cast<uint8_t>(subs.nn >> 8))
					) : (//need to throw error for when this is losing data cast
					addByte(static_cast<uint8_t>(subs.nn))
					);
			case 'O':
				addByte(subs.o);
				break;
			case 'T':
				shiftIntoLastByte(subs.t);
				break;
			case 'P':
				shiftIntoLastByte(subs.p);
				break;
			case '<':
				leftShifts++;
				break;
			default:
				hex();
			}
		}
		return std::move(bytes);
	}

	void FormatParser::Filler::addByte(uint8_t value)
	{
		bytes.push_back(value);
		readjustStart();
	}

	void FormatParser::Filler::shiftIntoLastByte(uint8_t value)
	{
		auto& lastByte = bytes.back();
		lastByte = lastByte | (value << leftShifts);
		leftShifts = 0;
		readjustStart();
	}

	void FormatParser::Filler::hex()
	{
		auto byteView = currentView();
		if (byteView.size == 2) {
			auto nextByte = hexToIntegral<uint8_t>({ byteView.data, byteView.size });
			addByte(nextByte);
		}
	}

	void FormatParser::Filler::reset()
	{
		StreamViewer<char>::reset(); 
		bytes.clear();
		leftShifts = 0;
	}

}