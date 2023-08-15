#pragma once
#include "Expr.h"
#include "spdlog/fmt/fmt.h"
#include <iostream>

namespace Expr 
{

class Printer
	: public ::Expr::Visitor {
public:
	Printer(int initialSpaces) 
		: currentSpaces(initialSpaces) {}
	Printer() = default;

	void printExpr(UniquePtr & expr) {
		this->visitChild(expr);
	}

	virtual void visit(Binary& expr) override {
		prettyPrint("{{lhs}} {} {{rhs}}", tokenTypeToStr(expr.oper)); 
		indentCallback([&]() { 
			printExpr(expr.lhs); 
			printExpr(expr.rhs); 
		}); 
	}
	virtual void visit(Unary& expr) override {

		prettyPrint("{} {{rhs}}", tokenTypeToStr(expr.oper));
		indentCallback([&]() {printExpr(expr.expr); });
	}
	//Primary expressions
	virtual void visit(Cast& expr) override {
		prettyPrint("{{expr}} as {{type}}");
		indentCallback([&]() {	
			printExpr(expr.expr); 
			printExpr(expr.type);
		});
	}
	virtual void visit(FunctionType& expr) override {
		prettyPrint("Function Type: {{return type}}({{1st}}, {{2nd}}, ... {{nth}})");
		indentCallback([&]() {
			printExpr(expr.returnType);
			for (auto& param : expr.paramTypes) {
				printExpr(param);
			}
		});
	}
	virtual void visit(KeyworkFunctionCall& expr) override 
	{
		prettyPrint("{}({{1st}}, {{2nd}}, ... {{nth}})", tokenTypeToStr(expr.function));
		indentCallback([&]() {
			for (auto& param : expr.args) {
				printExpr(param);
			}
		});
	}

	virtual void visit(Parenthesis& expr) override {
		prettyPrint("(  expr  )");
		indentCallback([&]() {printExpr(expr.expr); });
	}
	virtual void visit(Reference& expr) override {
		prettyPrint("{{expr}}&");
		indentCallback([&]() {printExpr(expr.expr); });
	}
	virtual void visit(Questionable& expr) override {
		prettyPrint("{{expr}}?");
		indentCallback([&]() { printExpr(expr.expr); });
	}
	virtual void visit(Identifier& expr) override {
		prettyPrint("{}",expr.ident);
	}
	virtual void visit(FunctionCall& expr) override {
		prettyPrint("Function Call: {{expr}}({{1st}}, {{2nd}}, ... {{nth}})");
		indentCallback([&]() {
			printExpr(expr.lhs); 
			for (auto& arg : expr.arguments) printExpr(arg);
		});
	}
	virtual void visit(TemplateCall& expr) override {
		prettyPrint("Template Call: {{expr}}<{{1st}}, {{2nd}}, ... {{nth}}>");
		indentCallback([&]() {
			printExpr(expr.lhs);
			for (auto& arg : expr.templateArgs) printExpr(arg);
		});
	}
	virtual void visit(Indexing& expr) override {
		prettyPrint("Indexing. {{lhs}} [{{rhs}}]");
		indentCallback([&]() {
			printExpr(expr.lhs); 
			printExpr(expr.innerExpr);
		});
	}
	virtual void visit(MemberAccess& expr) override {
		prettyPrint("Member Access: {{expr}}.", expr.member);
		indentCallback([&]() {printExpr(expr.lhs); });

	}
	virtual void visit(StructLiteral& expr) override {
		prettyPrint("Struct Literal: {{{{1st}}, {{2nd}}, ... {{nth}}}}"); 
		indentCallback([&]() {
			for (auto& init : expr.initializers) printExpr(init);
		});
	}
	virtual void visit(ListLiteral& expr) override {
		prettyPrint("List Literal: [{{1st}}, {{2nd}}, ... {{nth}}]");
		indentCallback([&]() {
			for (auto& init : expr.elements) printExpr(init);
		});
	}
	virtual void visit(Literal& expr) override {
		prettyPrint("{}", literalToStr(expr.literal));
	}
	virtual void visit(Register& expr) override {
		prettyPrint("Register: {}", expr.reg);
	}
	virtual void visit(Flag& expr) override {
		prettyPrint("Flag: {}", expr.flag);
	}
	virtual void visit(CurrentPC& expr) override {
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

}