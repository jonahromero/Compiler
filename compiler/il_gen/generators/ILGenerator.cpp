#include "ILGenerator.h"
#include "Parser.h"
#include "Lexer.h"
#include <variant>
#include "FunctionGenerator.h"
#include "VectorUtil.h"
#include "spdlog/spdlog.h"
#include "CFGGenerator.h"


ILGenerator::ILGenerator()
{
}

std::optional<IL::Program> ILGenerator::generate(Stmt::Program input)
{
	IL::Program ilProgram;
	for (auto& stmt : input) 
	{
		tryToCompile(stmt, ilProgram);
	}
	return isErroneous ? std::nullopt : std::make_optional(std::move(ilProgram));
}

void ILGenerator::tryToCompile(Stmt::UniquePtr& stmt, IL::Program& out)
{
	try {
		util::vector_append(out, visitStmt(stmt));
	}
	catch (SemanticError& error) {
		spdlog::error(error.toString());
		isErroneous = true;
	}
}

void ILGenerator::visit(Stmt::Bin& bin)
{
	env.types.addBin(std::move(bin));
	returnForStmt({});
}

void ILGenerator::visit(Stmt::Function& func)
{
	if (func.isTemplate()) {
		throw SemanticError(func.sourcePos, "Function templates not suppported");
		returnForStmt();
	}
	else {
		auto body = FunctionGenerator{ env }.generate(std::move(func));
		std::vector<IL::UniquePtr> stmts;
		stmts.push_back(IL::makeIL<IL::Function>(func.name, std::vector<IL::Decl>{}, IL::Type::i1, false, std::move(body)));
		returnForStmt(std::move(stmts));
	}
}
// handled by everything else
void ILGenerator::visit(Stmt::VarDef& varDef) {}
void ILGenerator::visit(Stmt::Assign& assign) {}
// control flow
void ILGenerator::visit(Stmt::Label& label) {}
void ILGenerator::visit(Stmt::CountLoop& loop) {}
void ILGenerator::visit(Stmt::If& ifStmt) {}
void ILGenerator::visit(Stmt::ExprStmt& exprStmt) {}
void ILGenerator::visit(Stmt::Return& stmt) {}
void ILGenerator::visit(Stmt::Instruction& stmt) {}
void ILGenerator::visit(Expr::Binary& expr) {}
void ILGenerator::visit(Expr::Unary& expr) {}
void ILGenerator::visit(Expr::StructLiteral& expr) {}
void ILGenerator::visit(Expr::ListLiteral& expr) {}
void ILGenerator::visit(Expr::Parenthesis& expr) {}
void ILGenerator::visit(Expr::Identifier& expr) {}
void ILGenerator::visit(Expr::MemberAccess& expr) {}
void ILGenerator::visit(Expr::Literal& expr) {}
void ILGenerator::visit(Expr::CurrentPC& expr) {}
void ILGenerator::visit(Expr::Register& expr) {}
void ILGenerator::visit(Expr::Flag& expr) {}
void ILGenerator::visit(Stmt::Module& mod) {}
void ILGenerator::visit(Stmt::Import& imp) {}
void ILGenerator::visit(Expr::FunctionCall& expr) {}
void ILGenerator::visit(Expr::TemplateCall& expr) {}
void ILGenerator::visit(Expr::Indexing& expr) {}