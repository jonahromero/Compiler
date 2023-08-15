#pragma once
#include "Token.h"
#include <memory>
#include "Visitors.h"

namespace Expr {

	class Expr
		: public visit::VisitableBase<Expr,
		class Logical, class Bitwise, class Comparison, class Equality, class Bitshift, class Term,
		class Unary, class Parenthesis, class Identifier, class Register, class Flag, class Literal,
		class CurrentPC> {};

	template<typename T>
	class VisitorReturner : public Expr::VisitorReturnerType<T> {};
	class Visitor : public Expr::VisitorType {};

	using UniquePtr = std::unique_ptr<Expr>;

	template<typename T, typename...Args>
	UniquePtr makeExpr(Args&&...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	namespace detail{
		struct BinaryExpr { UniquePtr lhs; Token::Type oper; UniquePtr rhs; };
	}

	struct Logical : Expr::Visitable<Logical>, detail::BinaryExpr {};
	struct Bitwise : Expr::Visitable<Bitwise>, detail::BinaryExpr {};
	struct Comparison : Expr::Visitable<Comparison>, detail::BinaryExpr {};
	struct Equality : Expr::Visitable<Equality>, detail::BinaryExpr {};
	struct Bitshift : Expr::Visitable<Bitshift>, detail::BinaryExpr {};
	struct Term : Expr::Visitable<Term>, detail::BinaryExpr {};

	struct Unary : Expr::Visitable<Unary> {
		Token::Type oper; 
		UniquePtr expr;
	};
	struct Parenthesis : Expr::Visitable<Parenthesis> {
		UniquePtr expr;
	};
	struct Identifier : Expr::Visitable<Identifier> {
		std::string_view ident;
	};
	struct Register : Expr::Visitable<Register> {
		std::string_view reg;
	};
	struct Flag : Expr::Visitable<Flag> {
		std::string_view flag;
	};
	struct Literal : Expr::Visitable<Literal> {
		Token::Literal literal;
	};
	struct CurrentPC : Expr::Visitable<CurrentPC> {};
}
/*
//keeping in case necessary?
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

}*/
