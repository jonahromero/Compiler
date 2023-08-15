#pragma once
#include "Expr.h"

#define printVisitTwoOperand(name) \
	virtual void visit(name& expr) { \
		prettyPrint("{lhs}", tokenTypeToStr(expr.oper), "{rhs}"); \
		indentCallback([&]() { \
			printExpr(expr.lhs); \
			printExpr(expr.rhs); \
		}); \
	}


class ExprPrinter
	: public Expr::ExprVisitor {
public:
	ExprPrinter(int initialSpaces) 
		: currentSpaces(initialSpaces) {}
	ExprPrinter() = default;

	void printExpr(Expr::UniquePtr & expr) {
		expr->accept(*this);
	}

	printVisitTwoOperand(Expr::Logical);
	printVisitTwoOperand(Expr::Bitwise);
	printVisitTwoOperand(Expr::Comparison);
	printVisitTwoOperand(Expr::Equality);
	printVisitTwoOperand(Expr::Bitshift);
	printVisitTwoOperand(Expr::Term);
	printVisitTwoOperand(Expr::Factor);

	virtual void visit(Expr::Unary& expr) {
		prettyPrint(tokenTypeToStr(expr.oper), "{rhs}");
		indentCallback([&]() {printExpr(expr.expr); });
	}
	//Primary expressions
	virtual void visit(Expr::Parenthesis& expr) {
		prettyPrint("(", "{expr}", ")");
		indentCallback([&]() {printExpr(expr.expr); });
	}
	virtual void visit(Expr::Identifier& expr) {
		prettyPrint(expr.ident);
	}
	virtual void visit(Expr::Literal& expr) {
		prettyPrint(literalToStr(expr.literal));
	}
	virtual void visit(Expr::Register& expr) {
		prettyPrint("Register", expr.reg);
	}
	virtual void visit(Expr::Flag& expr) {
		prettyPrint("Flag", expr.flag);
	}
	virtual void visit(Expr::CurrentPC& expr) {
		prettyPrint("$");
	}
protected:
	template<typename Callback>
	void indentCallback(Callback callback) {
		indent();
		callback();
		unindent();
	}

	template<typename...Args>
	void prettyPrint(Args&&...args) {
		printSpace();
		(((std::cout << std::forward<Args>(args)) << ' '), ...);
		std::cout << std::endl;
	}

	void printSpace() {
		for (size_t i = 0; i < currentSpaces; ++i) std::cout << ' ';
	}
	void indent() { currentSpaces += 4; }
	void unindent() { currentSpaces -= 4; }
	size_t currentSpaces = 0;
};