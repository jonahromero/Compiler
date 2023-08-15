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
#include "CtrlFlowGraph.h"

/*Will eventually support lang features*/
/* Error Handling:
	There can be errors while,
	Lexing: Handled in Compiler
	Parsing: Handled in Compiler
	Converting to Operands: Handled in Compiler
	Assembling Operands: Handled in Compiler
*/

class FunctionGenerator :
	public Stmt::VisitorReturner<IL::Program>
{
public:
	FunctionGenerator(Enviroment& env);
	IL::Function generate(Stmt::Function function);

private:
	Enviroment& env;

	void startFunctionEnviroment(std::string_view name, std::vector<Stmt::VarDecl> const& params, Expr::UniquePtr const& returnType);
	IL::Function::Signature createFunctionSignature(
		std::vector<Stmt::VarDecl> const& params, Expr::UniquePtr const& returnType,
		std::vector<TypeInstance>& compiledParamTypes, TypeInstance& compiledReturnType
	);

	ILCtrlFlowGraph transformGraph(CtrlFlowGraph graph);

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
};
