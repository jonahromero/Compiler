#pragma once
#include "Stmt.h"
#include "ILExprResult.h"


class ExprGenerator :
	public Expr::VisitorReturner<ILExprResult>
{
public:
	static ExprGenerator defaultContext(Enviroment& env);
	static ExprGenerator typedContext(Enviroment& env, IL::Type type);
	ILExprResult generate(Expr::UniquePtr& expr);
	ILExprResult generateWithCast(Expr::UniquePtr& expr, IL::Type outputType);

private:
	ExprGenerator(Enviroment& env, IL::Type arithmeticType);

	Enviroment& env;
	const IL::Type arithmeticType;

	// classifiers
	static bool isLogicalOperator(Token::Type oper);
	static bool isRelationalOperator(Token::Type oper);
	static bool isArithmeticOperator(Token::Type oper);
	static bool isPrimitiveType(TypePtr type);

	// Type Expressions
	struct UnaryTypeExprResult { IL::Type returnType, leftCast; };
	struct BinaryTypeExprResult { IL::Type returnType, leftCast, rightCast; };

	BinaryTypeExprResult generateBinaryTypeExprResult(SourcePosition pos, IL::Type left, Token::Type oper, IL::Type right);
	BinaryTypeExprResult generateUnaryTypeExprResult(SourcePosition pos, IL::Type left, Token::Type oper, IL::Type right);
	void assertValidCast(SourcePosition pos, IL::Type from, IL::Type to);

	// Helpers
	IL::Variable castVariable(IL::Program& instrs, SourcePosition pos, IL::Type type, IL::Variable var);
	IL::Variable derefVariable(IL::Program& instrs, IL::Variable var, PrimitiveType const* refType, bool isOpt);
	IL::Variable createNewILVariable(IL::Type type) const;
	
	void assertValidArgTypePassedToFunction(SourcePosition pos, TypeInstance param, TypeInstance arg) const;
	void assertCanPerformArithmeticWith(SourcePosition const& pos, TypeInstance const& type);

	TypeInstance performBinaryOperationWithTypes(SourcePosition pos, TypeInstance lhs, Token::Type oper, TypeInstance rhs) const;
	FunctionType const& expectCallable(Expr::UniquePtr const& expr);

	virtual void visit(Expr::Binary& expr);
	virtual void visit(Expr::Unary& expr);
	//Primary expressions
	virtual void visit(Expr::KeyworkFunctionCall& expr);
	virtual void visit(Expr::FunctionType& expr);
	virtual void visit(Expr::Cast& expr);
	virtual void visit(Expr::ListLiteral& expr);
	virtual void visit(Expr::StructLiteral& expr);
	virtual void visit(Expr::Questionable& expr);
	virtual void visit(Expr::Reference& expr);
	virtual void visit(Expr::Parenthesis& expr);
	virtual void visit(Expr::Identifier& expr);
	virtual void visit(Expr::FunctionCall& expr);
	virtual void visit(Expr::TemplateCall& expr);
	virtual void visit(Expr::Indexing& expr);
	virtual void visit(Expr::MemberAccess& expr);
	virtual void visit(Expr::Literal& expr);
	virtual void visit(Expr::Register& expr);
	virtual void visit(Expr::Flag& expr);
	virtual void visit(Expr::CurrentPC& expr);
};

