#pragma once
#include <cstdint>
#include <string>
#include "Stmt.h"
#include "Parser.h"
#include "Assembler.h"
#include "Operand.h"
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

struct NestingLocater {
	void enterLoop() { inLoop = true; }
	void exitLoop() { inLoop = false; }
	void enterFunction() { inGlobalScope = false; }
	void exitFunction() { inGlobalScope = true; }
	void enterConditional() { inIfStmt = true; }
	void exitConditional() { inIfStmt = false; }

	bool inGlobalScope = true;
	bool inIfStmt = false;
	bool inLoop = false;
};

class VariableCreator : public NestingLocater {
public:
	VariableCreator() {}

	auto createVariable()->IL::Variable { 
		return inGlobalScope ? IL::Variable(global++, true) : IL::Variable(variable++, false);
	}
	auto createLabel()->IL::Label { return IL::Label(label++); }
private:
	size_t global = 0, variable = 0, label = 0;
};

struct NodeBuilder : VariableCreator {
	void beginNode() {
		currentNode = entryNode = std::make_shared<IL::Node>(createLabel());
	}
	auto splitNode() -> std::pair<IL::NodePtr, IL::NodePtr> {
		auto firstLabel = createLabel(), secondLabel = createLabel();
		auto retval = std::make_pair(
			std::make_shared<IL::Node>(firstLabel),
			std::make_shared<IL::Node>(secondLabel)
		);
		currentNode->children.push_back(retval.first);
		currentNode->children.push_back(retval.second);
		return retval;
	}
	auto jumpToNewNode() -> IL::NodePtr {
		auto newNode = createNode();
		currentNode->children.push_back(newNode);
		currentNode->stmts.push_back(IL::makeIL<IL::Jump>(newNode->startLabel));
		currentNode = newNode;
		return newNode;
	}
	auto createNode() -> IL::NodePtr {
		return std::make_shared<IL::Node>(createLabel());
	}
	auto buildNode() -> IL::NodePtr {
		currentNode = nullptr;
		return std::move(entryNode);
	}
	IL::NodePtr entryNode, currentNode;
};

class ILGenerator : 
	public ExprStmtVisitor<std::vector<IL::UniquePtr>>,
	public NodeBuilder
{
public:
	ILGenerator();
	auto generate(Stmt::Program program) -> IL::Program;

private:
	TypeSystem typeSystem;
	Enviroment env;

	auto getILDestination(std::vector<IL::UniquePtr>& il)->IL::ILDestination;
	auto conditional(IL::NodePtr& testingNode, IL::NodePtr& endNode, Stmt::Conditional& condition)->IL::NodePtr; //returns false node

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

};

