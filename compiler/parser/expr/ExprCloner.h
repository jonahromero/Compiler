#pragma once
#include "Expr.h"
#include "spdlog/fmt/fmt.h"
#include <iostream>


namespace Expr 
{
	class Cloner
		: public Expr::CloneVisitor<Cloner> 
	{
	public:
		Cloner() = default;
		
		virtual void visit(Binary const& expr) override {
			returnCloned(expr, clone(expr.lhs), expr.oper, clone(expr.rhs));
		}
		virtual void visit(Unary const& expr) override {
			returnCloned(expr, expr.oper, clone(expr.expr));
		}
		//Primary expressions
		virtual void visit(KeyworkFunctionCall const& expr) override {
			returnCloned(expr, expr.function, cloneList(expr.args));
		}
		virtual void visit(FunctionType const& expr) override {
			returnCloned(expr, cloneList(expr.paramTypes), clone(expr.returnType));
		}
		virtual void visit(Cast const& expr) override {
			returnCloned(expr, clone(expr.expr), clone(expr.type));
		}
		virtual void visit(ListLiteral const& expr) override {
			returnCloned(expr, cloneList(expr.elements));
		}
		virtual void visit(StructLiteral const& expr) override {
			returnCloned(expr, cloneList(expr.initializers), expr.names);
		}
		virtual void visit(Reference const& expr) override {
			returnCloned(expr, clone(expr.expr));
		}
		virtual void visit(Questionable const& expr) override {
			returnCloned(expr, clone(expr.expr));
		}
		virtual void visit(Parenthesis const& expr) override {
			returnCloned(expr, clone(expr.expr));
		}
		virtual void visit(Identifier const& expr) override {
			returnCloned(expr, expr.ident);
		}
		virtual void visit(FunctionCall const& expr) override {
			returnCloned(expr, clone(expr.lhs), cloneList(expr.arguments));
		}
		virtual void visit(TemplateCall const& expr) override {
			returnCloned(expr, clone(expr.lhs), cloneList(expr.templateArgs));
		}
		virtual void visit(Indexing const& expr) override {
			returnCloned(expr, clone(expr.lhs), clone(expr.innerExpr));
		}
		virtual void visit(MemberAccess const& expr) override {
			returnCloned(expr, clone(expr.lhs), expr.member);
		}
		virtual void visit(Literal const& expr) override {
			returnCloned(expr, expr.literal);
		}
		virtual void visit(Register const& expr) override {
			returnCloned(expr, expr.reg);
		}
		virtual void visit(Flag const& expr) override {
			returnCloned(expr, expr.flag);
		}
		virtual void visit(CurrentPC const& expr) override {
			returnCloned(expr);
		}
	private:	
		template<typename T, typename...Args>
		void returnCloned(T const& expr, Args&&...new_args) const {
			returnValue(
				makeExpr<T>(
					expr.sourcePos, 
					std::forward<Args>(new_args)...
				)
			);
		}
	};
	
}