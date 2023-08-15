#pragma once
#include "Token.h"
#include <memory>
#include <string>
#include "Visitors.h"

namespace Expr {

	class Expr
		: public visit::VisitableBase<Expr,
		struct Logical, struct Bitwise, struct Comparison, struct Equality, struct Bitshift, struct Term,
		struct Factor, struct Unary, struct Parenthesis,
		struct Identifier, struct Register, struct Flag, struct Literal, struct CurrentPC> {
	public:
		Token::SourcePosition sourcePos;
	};

	template<typename T>
	class VisitorReturner : public Expr::VisitorReturnerType<T> {};
	class Visitor : public Expr::VisitorType {};

	using UniquePtr = std::unique_ptr<Expr>;

	template<typename T, typename...Args>
	UniquePtr makeExpr(Token::SourcePosition sourcePos, Args&&...args) {
		auto expr = std::make_unique<T>(T{ std::forward<Args>(args)... });
		expr->sourcePos = sourcePos;
		return expr;
	}

	namespace detail{
		struct BinaryExpr { 
			BinaryExpr(UniquePtr lhs, Token::Type oper, UniquePtr rhs) 
				: lhs(std::move(lhs)), oper(oper), rhs(std::move(rhs)) {}

			UniquePtr lhs; Token::Type oper; UniquePtr rhs; 
		};
	}

	struct Logical : Expr::Visitable<Logical>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };
	struct Bitwise : Expr::Visitable<Bitwise>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };
	struct Comparison : Expr::Visitable<Comparison>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };
	struct Equality : Expr::Visitable<Equality>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };
	struct Bitshift : Expr::Visitable<Bitshift>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };
	struct Term : Expr::Visitable<Term>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };
	struct Factor : Expr::Visitable<Factor>, detail::BinaryExpr { using BinaryExpr::BinaryExpr; };

	struct Unary : Expr::Visitable<Unary> {
		Unary(Token::Type oper, UniquePtr expr) 
			: oper(oper), expr(std::move(expr)) {}
		Token::Type oper; 
		UniquePtr expr; 
	};
	struct Parenthesis : Expr::Visitable<Parenthesis> {
		Parenthesis(UniquePtr expr)
			: expr(std::move(expr)) {}
		UniquePtr expr;
	};
	struct Identifier : Expr::Visitable<Identifier> {
		Identifier(std::string_view ident) : ident(ident) {}
		std::string_view ident;
	};
	struct Register : Expr::Visitable<Register> {
		Register(std::string_view reg) : reg(reg) {}
		std::string_view reg;
	};
	struct Flag : Expr::Visitable<Flag> {
		Flag(std::string_view flag) : flag(flag) {}
		std::string_view flag;
	};
	struct Literal : Expr::Visitable<Literal> {
		Literal(Token::Literal literal) : literal(literal) {}
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
