#pragma once
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "Token.h"
#include "Visitors.h"

namespace Expr 
{
	class Expr
		: public visit::VisitableBase<Expr,
		struct Binary, struct Unary, struct Identifier, struct Literal, 
		struct Parenthesis, struct FunctionCall, struct Indexing, struct MemberAccess,
		struct Register, struct Flag, struct CurrentPC, struct ListLiteral, struct StructLiteral,
		struct TemplateCall, struct Questionable, struct Reference, struct FunctionType, struct Cast,
		struct KeyworkFunctionCall
	> {
	public:
		SourcePosition sourcePos;
	};

	template<typename Derived>
	class CloneVisitor : public Expr::CloneVisitor<Derived> {};
	template<typename T>
	class VisitorReturner : public Expr::VisitorReturnerType<T> {};
	class Visitor : public Expr::VisitorType {};

	using UniquePtr = std::unique_ptr<Expr>;

	template<typename T, typename...Args>
	UniquePtr makeExpr(SourcePosition sourcePos, Args&&...args) {
		auto expr = std::make_unique<T>(T{ std::forward<Args>(args)... });
		expr->sourcePos = sourcePos;
		return expr;
	}

	struct Binary : Expr::Visitable<Binary> 
	{
		Binary(UniquePtr lhs, Token::Type oper, UniquePtr rhs)
			: lhs(std::move(lhs)), oper(oper), rhs(std::move(rhs)) {}

		UniquePtr lhs; Token::Type oper; UniquePtr rhs;
	};
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

	struct FunctionCall : Expr::Visitable<FunctionCall> {
		FunctionCall(UniquePtr lhs, std::vector<UniquePtr> args)
			: lhs(std::move(lhs)), arguments(std::move(args)) {}

		UniquePtr lhs;
		std::vector<UniquePtr> arguments;
	};

	struct KeyworkFunctionCall : Expr::Visitable<KeyworkFunctionCall>
	{
		KeyworkFunctionCall(Token::Type function, std::vector<UniquePtr> args)
			: function(function), args(std::move(args)) {}

		Token::Type function;
		std::vector<UniquePtr> args;
	};

	struct TemplateCall : Expr::Visitable<TemplateCall> {
		TemplateCall(UniquePtr lhs, std::vector<UniquePtr> templateArgs)
			: lhs(std::move(lhs)), templateArgs(std::move(templateArgs)) {}

		UniquePtr lhs;
		std::vector<UniquePtr> templateArgs;
	};
	struct Indexing : Expr::Visitable<Indexing> {
		Indexing(UniquePtr lhs, UniquePtr innerExpr)
			: lhs(std::move(lhs)), innerExpr(std::move(innerExpr)) {}

		UniquePtr lhs, innerExpr;
	};
	struct MemberAccess : Expr::Visitable<MemberAccess> {
		MemberAccess(UniquePtr lhs, std::string_view member)
			: lhs(std::move(lhs)), member(member) {}

		UniquePtr lhs;
		std::string_view member;
	};

	struct ListLiteral : Expr::Visitable<ListLiteral> 
	{
		ListLiteral(std::vector<UniquePtr> elements)
			: elements(std::move(elements)) {}
		std::vector<UniquePtr> elements;
	};

	struct StructLiteral : Expr::Visitable<StructLiteral> 
	{
		StructLiteral(std::vector<UniquePtr> initializers, std::optional<std::vector<std::string_view>> names = std::nullopt)
			: initializers(std::move(initializers)), names(std::move(names)) {}

		std::vector<UniquePtr> initializers;
		std::optional<std::vector<std::string_view>> names;
	};

	struct FunctionType : Expr::Visitable<FunctionType> 
	{
		FunctionType(std::vector<UniquePtr> paramTypes, UniquePtr returnType)
			:	paramTypes(std::move(paramTypes)), returnType(std::move(returnType)) {}

		std::vector<UniquePtr> paramTypes;
		UniquePtr returnType;
	};

	struct Cast : Expr::Visitable<Cast>
	{
		Cast(UniquePtr expr, UniquePtr type)
			: expr(std::move(expr)), type(std::move(type)) {}

		UniquePtr expr, type;
	};

	struct Questionable : Expr::Visitable<Questionable>
	{
		Questionable(UniquePtr expr) 
			: expr(std::move(expr)) {}

		UniquePtr expr;
	};

	struct Reference : Expr::Visitable<Reference> 
	{
		Reference(UniquePtr expr)
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