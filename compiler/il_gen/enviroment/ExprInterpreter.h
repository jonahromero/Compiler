#pragma once
#include "Expr.h"
#include <variant>
#include <string>
#include "IntTypes.h"
#include "TypeSystem.h"
#include "CompilerError.h"
#include "Enviroment.h"
#include "ComputedExpr.h"

class ExprInterpreter :
	public Expr::VisitorReturner<ComputedExpr>
{
public:
	ExprInterpreter(Enviroment& env);
	ComputedExpr interpret(Expr::UniquePtr const& expr);

	class NotConstEvaluable : public CompilerError {
	public:
		NotConstEvaluable(SourcePosition const& pos, Token::Type const& operation)
			: CompilerError(pos), operation(operation) {}

		NotConstEvaluable(SourcePosition const& pos, std::string what)
			: CompilerError(pos), what(what) {}

		std::string msgToString() {
			return fmt::format("Expression cannot be evaluated at compile-time: {}", 
				what.empty() ? tokenTypeToStr(operation) : what
			);
		}
	private:
		std::string what;
		Token::Type operation;
	};

	class InvalidOperation : public CompilerError {
	public:
		InvalidOperation(SourcePosition const& pos, Token::Type const& operation, ComputedExpr const& result)
			: CompilerError(pos), operation(operation), operation_target(result.typeToString()) {
		}

		std::string msgToString() {
			return fmt::format("Operation cannot be performed cannot on a {}: {}", operation_target, tokenTypeToStr(operation));
		}
	private:
		std::string_view operation_target;
		Token::Type operation;
	};

private:
	Enviroment& env;

	virtual void visit(Expr::Binary& expr);
	virtual void visit(Expr::Unary& expr);
	//Primary expressions

	virtual void visit(Expr::KeyworkFunctionCall& expr);
	virtual void visit(Expr::FunctionType& expr);
	virtual void visit(Expr::Cast& expr);
	virtual void visit(Expr::ListLiteral& expr);
	virtual void visit(Expr::StructLiteral& expr);
	virtual void visit(Expr::Reference& expr);
	virtual void visit(Expr::Questionable& expr);
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

	std::string_view compileTemplate(SourcePosition pos, TemplateBin const* type, std::vector<ComputedExpr> args);
	std::string createTemplateName(std::string_view templateID, std::vector<ComputedExpr> const& args);
	void assertCorrectTemplateArgs(SourcePosition pos, std::vector<TemplateBin::TemplateParam> const& params, std::vector<ComputedExpr> const& args);
	std::vector<Stmt::VarDecl> newBinBody(std::vector<Stmt::VarDecl> const& oldBody, std::vector<TemplateBin::TemplateParam> const& params, std::vector<ComputedExpr> const& args);
};
