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
#include "GeneratorToolKit.h"
#include "GeneratorErrors.h"
#include "FunctionHelpers.h"

/*Will eventually support lang features*/
/* Error Handling:
	There can be errors while,
	Lexing: Handled in Compiler
	Parsing: Handled in Compiler
	Converting to Operands: Handled in Compiler
	Assembling Operands: Handled in Compiler
*/

struct BlockStmtResult 
{
	BlockStmtResult(IL::Program allocations, IL::Program instructions) 
		: instructions(std::move(instructions)),
		  allocations(std::move(allocations)) {}
	IL::Program instructions, allocations;
};

class FunctionGenerator :
	public Stmt::VisitorReturner<BlockStmtResult>,
	public gen::GeneratorToolKit,
	public gen::GeneratorErrors
{
public:
	FunctionGenerator(Enviroment& env, IL::Program& moduleInstructions);
	IL::Function generate(Stmt::Function function);

private:
	Enviroment& env;
	IL::Program& moduleInstructions;
	std::optional<gen::Variable> returnVariable;

	ILCtrlFlowGraph transformGraph(CtrlFlowGraph graph);
	std::pair<IL::Function, std::vector<TypeInstance>> generate_(Stmt::Function function);

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
