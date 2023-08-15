#pragma once
#include "Expr.h"
#include <variant>
#include <string>
#include "TypeSystem.h"
#include "CompilerError.h"
#include "Enviroment.h"

using ExprResult = std::variant<std::string, u16, TypeSystem::TypeInstance>;

class ExprInterpreter :
	public Expr::VisitorReturner<ExprResult> 
{
public:
	ExprInterpreter(Enviroment& env, TypeSystem& typeSystem);
	virtual void visit(Expr::Binary& expr);
	virtual void visit(Expr::Unary& expr);
	//Primary expressions
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

	class NotConstEval : public CompilerError {
	public:
		NotConstEval(Token::SourcePosition const& pos, Token::Type const& operation)
			: CompilerError(pos), operation(operation) {}

		NotConstEval(Token::SourcePosition const& pos, std::string what)
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
		InvalidOperation(Token::SourcePosition const& pos, Token::Type const& operation, ExprResult const& result)
			: CompilerError(pos), operation(operation) {
			std::visit([&](auto&& arg) {
				using U = std::remove_cvref_t<decltype(arg)>;
				if constexpr (std::is_same_v<U, u16>) {
					operation_target = "integer";
				}
				else if constexpr (std::is_same_v<U,std::string>) {
					operation_target = "string";
				}
				else {
					operation_target = "type";
				}
			}, result);
		}

		std::string msgToString() {
			return fmt::format("Operation cannot be performed cannot on a {}: {}", operation_target, tokenTypeToStr(operation));
		}
	private:
		std::string_view operation_target;
		Token::Type operation;
	};

private:
	TypeSystem& typeSystem;
	Enviroment& env;

	static bool isIntegral(ExprResult const& result);
	static bool isString(ExprResult const& result);
	static bool isType(ExprResult const& result);
};
