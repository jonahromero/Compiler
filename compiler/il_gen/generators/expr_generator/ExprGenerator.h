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
	ExprGenerator(Enviroment& env, TypePtr arithmeticType);

	Enviroment& env;
	const TypePtr typeContext;

	// classifiers
	static bool isLogicalOperator(Token::Type oper);
	static bool isRelationalOperator(Token::Type oper);
	static bool isArithmeticOperator(Token::Type oper);
	static bool isPrimitiveType(TypePtr type);

	IL::Type getReferenceTypeImplementation(gen::ReferenceType refType, size_t size);
	gen::Variable allocateVariable(IL::Program& instructions, TypeInstance type);
	// Helpers
	IL::Variable simpleAllocate(IL::Program& instrs, size_t size);
	IL::Variable simpleDeref(IL::Program& instrs, IL::Type type, IL::Variable ptr);
	IL::Variable addToPointer(IL::Program& instrs, IL::Variable ptr, int offset);
	IL::Variable castVariable(IL::Program& instrs, IL::Type type, IL::Variable var);
	IL::Variable derefVariable(IL::Program& instrs, IL::Variable var, IL::ReferenceType refType, PrimitiveType const* refType, bool isOpt);
	
	IL::Variable createNewILVariable(IL::Type type) const;
	
	void assertValidFunctionArgType(SourcePosition pos, TypeInstance param, TypeInstance arg) const;
	void assertIsAssignableType(SourcePosition pos, TypeInstance dest, TypeInstance src) const;
	void assertValidListLiteral(SourcePosition pos, std::vector<TypeInstance> const& elementTypes, TypeInstance expected) const;
	void assertCorrectFunctionCall(SourcePosition pos, std::vector<TypeInstance> const& params, std::vector<TypeInstance> const& args);

	PrimitiveType const& expectPrimitive(SourcePosition const& pos, TypeInstance const& type);
	FunctionType const& expectCallable(SourcePosition const& pos, TypeInstance const& type);
	BinType::Field const& expectMember(SourcePosition const& pos, TypeInstance const& type, std::string_view name);

	// binary helpers
	PrimitiveType const& determineBinaryOperandCasts(SourcePosition const& pos, 
													 PrimitiveType const& lhs, 
													 Token::Type oper, 
													 PrimitiveType const& rhs);
	
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

