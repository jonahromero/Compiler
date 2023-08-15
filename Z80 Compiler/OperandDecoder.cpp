#include "OperandDecoder.h"
#include "VariantUtil.h"

namespace Asm {
	using std::holds_alternative;

	void OperandDecoder::visit(Expr::Binary& expr)
	{
		if (expr.oper == PLUS || expr.oper == MINUS) {
			auto lhs = decodeInnerExpr(expr.lhs);
			auto rhs = decodeInnerExpr(expr.rhs);
			uint16_t sign = expr.oper == PLUS ? 1 : -1;

			std::visit(OverloadVariant{
				[&](Number& offset, Register& reg) {
					returnValue(OffsetRegister(reg.str, sign * offset.val));
				},
				[&](Register& reg, Number& offset) {
					returnValue(OffsetRegister(reg.str, sign * offset.val));
				},
				[&](Number& lhs, Number& rhs) {
					returnValue(Number(lhs.val + sign * rhs.val));
				},
				[](auto, auto) {
					throw InvalidRegisterAddition();
				}
			}, lhs, rhs);
		}
		else{
			auto [lhs, rhs] = getOperandsFromBinaryExpr(expr);
			switch (expr.oper) {
				case STAR:			returnValue(Number(lhs * rhs)); break;
				case SLASH:			returnValue(Number(lhs / rhs)); break;
				case MODULO:		returnValue(Number(lhs % rhs)); break;
				case SHIFT_LEFT:	returnValue(Number(lhs << rhs)); break;
				case SHIFT_RIGHT:	returnValue(Number(lhs >> rhs)); break;
				case EQUAL:			returnValue(Number(lhs == rhs)); break;
				case NOT_EQUAL:		returnValue(Number(lhs != rhs)); break;
				case LESS:			returnValue(Number(lhs < rhs)); break;
				case LESS_EQUAL:	returnValue(Number(lhs <= rhs)); break;
				case GREATER:		returnValue(Number(lhs > rhs)); break;
				case GREATER_EQUAL:	returnValue(Number(lhs >= rhs)); break;
				case BIT_AND:		returnValue(Number(lhs & rhs)); break;
				case BIT_OR:		returnValue(Number(lhs | rhs)); break;
				case BIT_XOR:		returnValue(Number(lhs ^ rhs)); break;
				case AND:			returnValue(Number(lhs && rhs)); break;
				case OR: 			returnValue(Number(lhs || rhs)); break;
			}
		}
	}

	void OperandDecoder::visit(Expr::Unary& expr)
	{
		auto lhs = evalIntegerExpr(expr.expr);
		switch (expr.oper) {
		case MINUS:
			returnValue(Number(-lhs)); break;
		case BANG:
			returnValue(Number(!lhs)); break;
		case BIT_NOT:
			returnValue(Number(~lhs)); break;
		}
	}

	void OperandDecoder::visit(Expr::Parenthesis& expr)
	{
		Operand inner = decodeInnerExpr(expr.expr);
		if (!isInnerExpr) {
			returnValue(Dereference{ expectDereferenceable(inner) });
		}
		else {
			returnValue(inner);
		}
	}

	void OperandDecoder::visit(Expr::Literal& expr)
	{
		if (!holds_alternative<uint16_t>(expr.literal)) {
			throw ExpectedInteger();
		}
		returnValue(std::get<uint16_t>(expr.literal));
	}

	void OperandDecoder::visit(Expr::Identifier& expr)
	{
		returnValue(labelContext.at(expr.ident));
	}

	void OperandDecoder::visit(Expr::Register& reg)
	{
		returnValue(Register{reg.reg});
	}

	void OperandDecoder::visit(Expr::Flag& flag)
	{
		returnValue(Flag{flag.flag});
	}

	void OperandDecoder::visit(Expr::CurrentPC& expr)
	{
		returnValue(pc);
	}

	uint16_t OperandDecoder::expectInteger(Operand oper)
	{
		if (!holds_alternative<Number>(oper)) {
			throw ExpectedInteger();
		}
		return std::get<Number>(oper).val;
	}

	auto OperandDecoder::expectDereferenceable(Operand oper) -> Dereferenceable
	{
		return std::visit(OverloadVariant{
			[](Number& num) {
				return Dereferenceable(num);
			},
			[](Register& reg) {
				return Dereferenceable(reg);
			},
			[](OffsetRegister& offsetReg) {
				return Dereferenceable(offsetReg);
			},
			[](auto) {
				throw ExpectedDereferenceable();
				return Dereferenceable{0}; 
			}
		}, oper);
	}
}