#pragma once
#include <cstdint>
#include <string>
#include "Stmt.h"
#include "Parser.h"
#include "TypeSystem.h"
#include "IL.h"
#include "Enviroment.h"
#include "VectorUtil.h"
#include "ExprStmtVisitor.h"

/*Will eventually support lang features*/
/* Error Handling:
	There can be errors while, 
	Lexing: Handled in Compiler
	Parsing: Handled in Compiler
	Converting to Operands: Handled in Compiler
	Assembling Operands: Handled in Compiler
*/


class ILGenerator final : 
	public ExprStmtVisitor<std::vector<IL::UniquePtr>>
{
public:
	ILGenerator();
	std::optional<IL::Program> generate(Stmt::Program program);
private:
	void tryToCompile(Stmt::UniquePtr& stmt, IL::Program& out);

	Enviroment env;
	bool isErroneous = false;

	virtual void visit(Stmt::Bin& bin) override;
	virtual void visit(Stmt::Module& mod) override;
	virtual void visit(Stmt::Import& imp) override;
	virtual void visit(Stmt::Function& func) override;

	virtual void visit(Stmt::VarDef& varDef) override;
	virtual void visit(Stmt::CountLoop& loop) override;
	virtual void visit(Stmt::Assign& assign) override;
	virtual void visit(Stmt::If& ifStmt) override;
	virtual void visit(Stmt::ExprStmt& exprStmt) override;
	virtual void visit(Stmt::Return& stmt) override;
	virtual void visit(Stmt::Label& label) override;
	virtual void visit(Stmt::Instruction& stmt) override;
	virtual void visit(Stmt::NullStmt& nullStmt) override {} //do nothing

	virtual void visit(Expr::Binary& expr) override;
	virtual void visit(Expr::Unary& expr) override;
	//Primary expressions
	virtual void visit(Expr::KeyworkFunctionCall& expr) override {}
	virtual void visit(Expr::FunctionType& expr) override {}
	virtual void visit(Expr::Cast& expr) override {}
	virtual void visit(Expr::StructLiteral& expr) override;
	virtual void visit(Expr::ListLiteral& expr) override;
	virtual void visit(Expr::Reference& expr) override {}
	virtual void visit(Expr::Questionable& expr) override {}
	virtual void visit(Expr::Parenthesis& expr) override;
	virtual void visit(Expr::Identifier& expr) override;
	virtual void visit(Expr::FunctionCall& expr) override;
	virtual void visit(Expr::TemplateCall& expr) override;
	virtual void visit(Expr::Indexing& expr) override;
	virtual void visit(Expr::MemberAccess& expr) override;
	virtual void visit(Expr::Literal& expr) override;
	virtual void visit(Expr::Register& expr) override;
	virtual void visit(Expr::Flag& expr) override;
	virtual void visit(Expr::CurrentPC& expr) override;
};

