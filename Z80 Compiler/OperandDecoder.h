#pragma once
#include <optional>
#include <unordered_map>
#include "Expr.h"
#include "ExpectationErrors.h"
#include "Operand.h"

/*Label Context */
namespace Asm {

	class OperandDecoder
		: public Expr::VisitorReturner<Operand> {
	public:
		using enum Token::Type;

		class ExpectedInteger {};
		class ExpectedDereferenceable {};
		class InvalidRegisterAddition {};

		OperandDecoder(LabelContext const& labelContext, uint16_t pc)
			: labelContext(labelContext), pc(pc) {}

		Operand decodeExpr(Expr::UniquePtr& expr) {
			isInnerExpr = false;
			return visitPtr(expr);
		}
		Operand decodeInnerExpr(Expr::UniquePtr& expr) {
			bool prevExprDepth = isInnerExpr;
			isInnerExpr = true;
			Operand retval = visitPtr(expr);
			isInnerExpr = prevExprDepth;
			return retval;
		}
		virtual void visit(Expr::Logical& expr);
		virtual void visit(Expr::Bitwise& expr);
		virtual void visit(Expr::Comparison& expr);
		virtual void visit(Expr::Equality& expr);
		virtual void visit(Expr::Bitshift& expr);
		virtual void visit(Expr::Term& expr);
		virtual void visit(Expr::Factor& expr);
		virtual void visit(Expr::Unary& expr);
		//Primary expressions
		virtual void visit(Expr::Parenthesis& expr);
		virtual void visit(Expr::Identifier& expr);
		virtual void visit(Expr::Register& reg);
		virtual void visit(Expr::Flag& flag);
		virtual void visit(Expr::Literal& expr);
		virtual void visit(Expr::CurrentPC& expr);
	private:
		template<typename ExprType>
		auto getOperandsFromBinaryExpr(ExprType& expr) -> std::pair<uint16_t, uint16_t> {
			return std::pair(evalIntegerExpr(expr.lhs), evalIntegerExpr(expr.rhs));
		}
		uint16_t evalIntegerExpr(Expr::UniquePtr& expr) {
			return expectInteger(decodeInnerExpr(expr));
		}
		static uint16_t expectInteger(Operand oper);
		static auto expectDereferenceable(Operand oper)-> Dereferenceable;

		uint16_t pc;
		bool isInnerExpr;
		LabelContext const& labelContext;
	};
}