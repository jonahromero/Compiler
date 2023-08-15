#pragma once
#include "Expr.h"
#include "spdlog/fmt/fmt.h"
#include <iostream>

class ExprPrinter
	: public Expr::Visitor {
public:
	ExprPrinter(int initialSpaces) 
		: currentSpaces(initialSpaces) {}
	ExprPrinter() = default;

	void printExpr(Expr::UniquePtr & expr) {
		this->visitPtr(expr);
	}

	virtual void visit(Expr::Binary& expr) {
		prettyPrint("{{lhs}} {} {{rhs}}", tokenTypeToStr(expr.oper)); 
		indentCallback([&]() { 
			printExpr(expr.lhs); 
			printExpr(expr.rhs); 
		}); 
	}
	virtual void visit(Expr::Unary& expr) {

		prettyPrint("{} {{rhs}}", tokenTypeToStr(expr.oper));
		indentCallback([&]() {printExpr(expr.expr); });
	}
	//Primary expressions
	virtual void visit(Expr::Parenthesis& expr) {
		prettyPrint("(  expr  )");
		indentCallback([&]() {printExpr(expr.expr); });
	}
	virtual void visit(Expr::Identifier& expr) {
		prettyPrint("{}",expr.ident);
	}
	virtual void visit(Expr::FunctionCall& expr) {
		prettyPrint("Function Call: {{expr}}({{1st}}, {{2nd}}, ... {{nth}})");
		indentCallback([&]() {
			printExpr(expr.lhs); 
			for (auto& arg : expr.arguments) printExpr(arg);
		});
	}
	virtual void visit(Expr::Indexing& expr) {
		prettyPrint("Indexing. {{lhs}} [{{rhs}}]");
		indentCallback([&]() {
			printExpr(expr.lhs); 
			printExpr(expr.innerExpr);
		});
	}
	virtual void visit(Expr::MemberAccess& expr) {
		prettyPrint("Member Access: {{expr}}.", expr.member);
		indentCallback([&]() {printExpr(expr.lhs); });

	}
	virtual void visit(Expr::Literal& expr) {
		prettyPrint("{}", literalToStr(expr.literal));
	}
	virtual void visit(Expr::Register& expr) {
		prettyPrint("Register: {}", expr.reg);
	}
	virtual void visit(Expr::Flag& expr) {
		prettyPrint("Flag: {}", expr.flag);
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
	constexpr void prettyPrint(std::string_view fmt, Args&&...args) {
		printSpace();
		std::cout << fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...));
		std::cout << std::endl;
	}

	void printSpace() {
		for (size_t i = 0; i < currentSpaces; ++i) std::cout << ' ';
	}
	void indent() { currentSpaces += 4; }
	void unindent() { currentSpaces -= 4; }
	size_t currentSpaces = 0;
};