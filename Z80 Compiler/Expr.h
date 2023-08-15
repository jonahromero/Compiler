#pragma once
#include "Token.h"
#include <memory>
#include "VisitorReturner.h"

namespace Expr {

	class Expr {
	public:
		void accept(class Visitor& visitor) {
			doAccept(visitor);
		}
		template<typename> class VisitorReturner;
		template<typename T>
		T accept(class VisitorReturner<T>& visitor);
	private:
		virtual void doAccept(Visitor& visitor) = 0;
	};

	using UniquePtr = std::unique_ptr<Expr>;

	template<typename T, typename...Args>
	UniquePtr makeExpr(Args&&...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	class Visitor {
	public:
		virtual void visit(class Logical& expr) = 0;
		virtual void visit(class Bitwise& expr) = 0;
		virtual void visit(class Comparison& expr) = 0;
		virtual void visit(class Equality& expr) = 0;
		virtual void visit(class Bitshift& expr) = 0;
		virtual void visit(class Term& expr) = 0;
		virtual void visit(class Factor& expr) = 0;
		virtual void visit(class Unary& expr) = 0;
		//Primary expressions
		virtual void visit(class Parenthesis& expr) = 0;
		virtual void visit(class Literal& expr) = 0;
		virtual void visit(class CurrentPC& expr) = 0;
		virtual void visit(class Identifier& expr) = 0;
		virtual void visit(class Register& expr) = 0;
		virtual void visit(class Flag& expr) = 0;
	};

	template<typename ReturnType>
	class VisitorReturner
		: public ::VisitorReturner<ReturnType>,
		private Visitor
	{
	public:
		ReturnType visitAndReturn(UniquePtr& expr) {
			return expr->accept<decltype(*this)>(*this);
		}
	};

	template<typename T>
	inline T Expr::accept(VisitorReturner<T>& visitor)
	{
		doAccept(visitor);
		return visitor.flushRetval();
	}

	//ICKY ICKY MACROS
	//defines all the expression classes
	//with proper visit functions and members

#define PREPROCCESOR_COMMA ,

#define ExprType(Name, Members, Ctor) \
	class Name : public Expr { \
	public: \
		Name Ctor {}\
		Members; \
		virtual void doAccept(Visitor& visitor) override { \
			visitor.visit(*this); \
		} \
	} \

#define TwoOperandExprType(Name) \
	ExprType(Name, UniquePtr lhs; Token::Type oper; UniquePtr rhs, \
		(UniquePtr lhs PREPROCCESOR_COMMA Token::Type oper PREPROCCESOR_COMMA UniquePtr rhs) \
		: lhs(std::move(lhs)) PREPROCCESOR_COMMA oper(oper) PREPROCCESOR_COMMA rhs(std::move(rhs)) \
	)

	TwoOperandExprType(Logical);
	TwoOperandExprType(Bitwise);
	TwoOperandExprType(Comparison);
	TwoOperandExprType(Equality);
	TwoOperandExprType(Bitshift);
	TwoOperandExprType(Term);
	TwoOperandExprType(Factor);

	ExprType(Unary, Token::Type oper; UniquePtr expr,
		(Token::Type oper PREPROCCESOR_COMMA UniquePtr expr) : expr(std::move(expr)) PREPROCCESOR_COMMA oper(oper)
	);
	//Primary classes
	ExprType(Parenthesis, UniquePtr expr,
		(UniquePtr expr) : expr(std::move(expr))
	);
	ExprType(Identifier, std::string_view ident,
		(std::string_view ident) : ident(ident)
	);
	ExprType(Register, std::string_view reg,
		(std::string_view reg) : reg(reg)
	);
	ExprType(Flag, std::string_view flag,
		(std::string_view flag) : flag(flag)
	);
	ExprType(Literal, Token::Literal literal,
		(Token::Literal literal) : literal(literal)
	);
	ExprType(CurrentPC, ,
		()
	);

}
