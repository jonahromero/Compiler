#pragma once
#include "Expr.h"
#include "spdlog/fmt/fmt.h"
#include <iostream>


namespace Expr 
{
	class Cloner
		: public Expr::CloneVisitor<Cloner, Expr::UniquePtr> 
	{
	public:
		ExprCopier() = default;
		
		virtual void visit(Expr::Binary& expr) {
			returnCloned(expr, clone(expr.lhs), expr.oper, clone(expr.rhs));
		}
		virtual void visit(Expr::Unary& expr) {
			returnCloned(expr, expr.oper, clone(expr.expr));
		}
		//Primary expressions
		virtual void visit(Expr::Parenthesis& expr) {
			returnCloned(expr, clone(expr.expr));
		}
		virtual void visit(Expr::Identifier& expr) {
			returnCloned(expr, expr.ident);
		}
		virtual void visit(Expr::FunctionCall& expr) {
			returnCloned(expr, clone(expr.lhs), cloneList(expr.arguments));
		}
		virtual void visit(Expr::TemplateCall& expr) {
			returnCloned(expr, clone(expr.lhs), cloneList(templateArgs), expr.isMut);
		}
		virtual void visit(Expr::Indexing& expr) {
			returnCloned(expr, clone(expr.lhs), clone(expr.innerExpr));
		}
		virtual void visit(Expr::MemberAccess& expr) {
			returnCloned(expr, clone(expr.lhs), expr.member);
		}
		virtual void visit(Expr::Literal& expr) {
			returnCloned(expr, expr.literal);
		}
		virtual void visit(Expr::Register& expr) {
			returnCloned(expr, expr.reg);
		}
		virtual void visit(Expr::Flag& expr) {
			returnCloned(expr, expr.flag);
		}
		virtual void visit(Expr::CurrentPC& expr) {
			returnCloned(expr);
		}
	private:	
		template<typename T, typename...Args>
		void returnCloned(T const& expr, Args&&...new_args) {
			returnValue(
				Expr::makeExpr<T>(
					expr.sourcePos, 
					std::forward<Args>(new_args)...
				)
			);
		}
	};
	
}