
#pragma once
#include <span>
#include "Stmt.h"
#include "SemanticError.h"
#include "CFG.h"


class CFGGenerator :
	public Stmt::Visitor, public Expr::Visitor {
public:
	CFGGenerator(Stmt::StmtBody& body) : body(body) {}

	void generate() {
		auto exitNode = visitStmts(cfg.getEntryNode(), body);
		cfg.addEdge(exitNode, cfg.getExitNode());
	}
private:
	// build tree starting from entryNode with statements, with entry block specified with branch type
	// returns the last cfg node that was created
	size_t visitStmts(size_t entryNode, Stmt::StmtBody& stmts, CFGBlock::BranchType branchType = CFGBlock::BranchType::NONE) {
		lastEntry.push(entryNode);
		currentBlock.branchType = branchType;
		if(stmts.empty()) {
			auto emptyNode = finishNode();
		}
		else {
			currentBlock.branchType = branchType;
			for(auto& stmt : stmts) {
				Stmt::Visitor::visitPtr(stmt);
			}
		}
		auto exitNode = lastEntry.top();
		lastEntry.pop();
		return exitNode;
	}

	Stmt::StmtBody& body;
    CFGBlock currentBlock;
	std::stack<size_t> lastEntry;
	CFG cfg;

	virtual void visit(Stmt::Bin& bin) override { 
        throw SemanticError(bin.sourcePos, "Unable to declare a bin within function body.");
    }
	virtual void visit(Stmt::Module& mod) override {
        throw SemanticError(bin.sourcePos, "Unable to declare a module within function body.");
    } 
	virtual void visit(Stmt::Import& imp) override {
        throw SemanticError(bin.sourcePos, "Unable to import a bin within function body.");
    }
	virtual void visit(Stmt::Function& func) override {
        throw SemanticError(bin.sourcePos, "Unable to nest functions within one another.");
    }
	virtual void visit(Stmt::CountLoop& loop) override {
		consumeStmt(Stmt::makeStmt<Stmt::VarDef>(loop.sourcePos,
			VarDecl(loop.counter, Expr::makeExpr<Expr::Identifier>(loop.sourcePos, "")), 
			false, loop.initializer
		));
		auto initializerBlock = finishNode();
		visitStmts(initializerBlock, loop.body, CFGBlock::BranchType::TRUE_BRANCH);
		currentBlock.branchType = CFGBlock::BranchType::FALSE_BRANCH;
	}

	virtual void visit(Stmt::If& ifStmt) override {
		visitConditionals(ifStmt.ifBranch, ifStmt.elseIfBranch, ifStmt.elseBranch);
	}

	virtual void visit(Stmt::Assign& assign) override {
		consumeStmt(std::move(assign));
    }
	virtual void visit(Stmt::ExprStmt& exprStmt) override {
		consumeStmt(std::move(exprStmt));
	}
	virtual void visit(Stmt::Return& stmt) override {
		consumeStmt(std::move(stmt));
		auto newNode = finishNode();
		cfg.addEdge(newNode, cfg.getExitNode());
	}
	virtual void visit(Stmt::VarDef& varDef) override { 
		consumeStmt(std::move(varDef));
	}

	virtual void visit(Stmt::Label& label) override {
		finishNode();
		currentBlock.label = label.label;
	}
	virtual void visit(Stmt::Instruction& stmt) override {
		consumeStmt(std::move(stmt));
	}

	virtual void visit(Stmt::NullStmt& nullStmt) override {} //do nothing

	size_t visitConditionals(Stmt::Conditional& trueBlock, std::span<Stmt::Conditional> conditionals, Stmt::Conditional& elseBranch) 
	{
		size_t falseExit;
		consumeStmt<Stmt::ExprStmt>(std::move(trueBlock.expr));
		auto startNode = finishNode();
		auto trueExit = visitStmts(startNode, ifStmt.body, CFGBlock::BranchType::TRUE_BRANCH);
		if(!conditionals.empty())
		{
			auto& falseBlock = conditionals[0];
			currentBlock.branchType = CFGBlock::BranchType::FALSE_BRANCH;
			consumeStmt<Stmt::ExprStmt>(std::move(falseBlock.expr));
			finishNode();
			falseExit = visitConditionals(falseBlock, conditionals.subspan(1), elseBranch);
		}
		else
		{
			falseExit = visitStmts(startNode, elseBranch, CFGBlock::BranchType::FALSE_BRANCH);
		}
		auto joinNode = visitStmts(falseExit, currentBlock);
		cfg.addEdge(trueExit, joinNode);
		return joinNode;
	}


	template<typename T>
	void consumeStmt(std::remove_cvref_t<T>&& stmt) {
        currentBlock.body.emplace_back(Stmt::makeStmt<std::remove_cvref_t<T>>(stmt.sourcePos, std::move(stmt)));
	}
	size_t finishNode() {
		auto newNode = cfg.createNode(std::move(currentBlock));
		cfg.addEdge(lastEntry.top(), newNode);
		lastEntry.top() = newNode;
		return newNode;
	}
};