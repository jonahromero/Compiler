#pragma once
#include "Expr.h"
#include "TypingSystem.h"
#include "Enviroment.h"

class SemanticAnalyzer :
	public Expr::VisitorReturner<TypingSystem::Type> {
public:
	SemanticAnalyzer(Enviroment enviroment)
		: enviroment(enviroment) {}
private:
	Enviroment enviroment;

	virtual void visit(Expr::Literal& expr) {}
	virtual void visit(Expr::Register& expr) {}
	virtual void visit(Expr::Flag& expr) {}
	virtual void visit(Expr::CurrentPC& expr) {}

	printVisitTwoOperand(Expr::Logical);
	printVisitTwoOperand(Expr::Bitwise);
	printVisitTwoOperand(Expr::Comparison);
	printVisitTwoOperand(Expr::Equality);
	printVisitTwoOperand(Expr::Bitshift);
	printVisitTwoOperand(Expr::Term);
	printVisitTwoOperand(Expr::Factor);

	virtual void visit(Expr::Unary& expr) {}
	//Primary expressions
	virtual void visit(Expr::Parenthesis& expr) {}
	virtual void visit(Expr::Identifier& expr) {}
	virtual void visit(Expr::FunctionCall& expr) {}
	virtual void visit(Expr::Indexing& expr) {}
	virtual void visit(Expr::MemberAccess& expr) {}

};