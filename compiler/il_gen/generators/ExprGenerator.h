#pragma once
#include "Stmt.h"
#include "ILExprResult.h"
#include "Generator.h"
#include "GeneratorErrors.h"


class ExprGenerator :
	public Expr::ConstVisitorReturner<ILExprResult>,
	public gen::Generator,
	public gen::GeneratorErrors
{
public:
	static ExprGenerator defaultContext(Enviroment& env);
	static ExprGenerator typedContext(Enviroment& env, TypePtr type);
	ILExprResult generate(Expr::UniquePtr const& expr);
	ILExprResult generateWithCast(Expr::UniquePtr const& expr, TypeInstance outputType);

private:
	ExprGenerator(Enviroment& env, TypePtr arithmeticType);

	Enviroment& env;
	const TypePtr typeContext;

	// classifiers
	static bool isLogicalOperator(Token::Type oper);
	static bool isRelationalOperator(Token::Type oper);
	static bool isArithmeticOperator(Token::Type oper);
	
	// binary helpers
	PrimitiveType const* determineBinaryOperandCasts(SourcePosition const& pos, 
													 PrimitiveType const* lhs, 
													 Token::Type oper, 
													 PrimitiveType const* rhs);
	TypeInstance determineBinaryReturnType(Token::Type oper, TypeInstance defaultType) const;



	virtual void visit(Expr::Binary const& expr);
	virtual void visit(Expr::Unary const& expr);
	//Primary expressions
	virtual void visit(Expr::KeyworkFunctionCall const& expr);
	virtual void visit(Expr::FunctionType const& expr);
	virtual void visit(Expr::Cast const& expr);
	virtual void visit(Expr::ListLiteral const& expr);
	virtual void visit(Expr::StructLiteral const& expr);
	virtual void visit(Expr::Questionable const& expr);
	virtual void visit(Expr::Reference const& expr);
	virtual void visit(Expr::Parenthesis const& expr);
	virtual void visit(Expr::Identifier const& expr);
	virtual void visit(Expr::FunctionCall const& expr);
	virtual void visit(Expr::TemplateCall const& expr);
	virtual void visit(Expr::Indexing const& expr);
	virtual void visit(Expr::MemberAccess const& expr);
	virtual void visit(Expr::Literal const& expr);
	virtual void visit(Expr::Register const& expr);
	virtual void visit(Expr::Flag const& expr);
	virtual void visit(Expr::CurrentPC const& expr);
};

