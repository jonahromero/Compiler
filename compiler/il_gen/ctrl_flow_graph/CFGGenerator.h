
#pragma once
#include <span>
#include "Stmt.h"
#include "SemanticError.h"
#include "CtrlFlowGraph.h"
#include "ExprCloner.h"


class CtrlFlowGraphGenerator :
	public Stmt::Visitor 
{
public:
	CtrlFlowGraphGenerator(Stmt::StmtBody& body) : body(body) {}

	CtrlFlowGraph generate() 
	{
		auto startNode = cfg.createNode(Block::defaultBlock());
		cfg.addEdge(cfg.getEntryNode(), startNode);
		auto lastBodyNode = visitStmts(startNode, body);
		cfg.addEdge(lastBodyNode, cfg.getExitNode());
		// fix return blocks
		for (auto returnBlock : returnBlocks) {
			cfg.removeAllEdges(returnBlock);
			cfg.addEdge(returnBlock, cfg.getExitNode());
		}
		return std::move(cfg);
	}
private:
	// build tree starting from entryNode with statements
	// returns the last cfg node that was created
	size_t visitStmts(size_t entryNode, Stmt::StmtBody& stmts) 
	{
		lastEntry.push(entryNode);
		if(stmts.empty()) {
			updateCurrentEntry(createChild());
		}
		else {
			for(auto& stmt : stmts) {
				visitChild(stmt);
			}
		}
		auto exitNode = lastEntry.top();
		lastEntry.pop();
		return exitNode;
	}

	CtrlFlowGraph cfg;
	Stmt::StmtBody& body;
	std::stack<size_t> lastEntry;
	std::vector<size_t> returnBlocks;

	virtual void visit(Stmt::Bin& bin) override { 
        throw SemanticError(bin.sourcePos, "Unable to declare a bin within function body.");
    }
	virtual void visit(Stmt::Module& mod) override {
        throw SemanticError(mod.sourcePos, "Unable to declare a module within function body.");
    } 
	virtual void visit(Stmt::Import& imp) override {
        throw SemanticError(imp.sourcePos, "Unable to import a bin within function body.");
    }
	virtual void visit(Stmt::Function& func) override {
        throw SemanticError(func.sourcePos, "Unable to nest functions within one another.");
    }
	virtual void visit(Stmt::CountLoop& loop) override {

		Stmt::UniquePtr varInitializer = Stmt::makeStmt<Stmt::VarDef>(loop.sourcePos,
			Stmt::GenericDecl{ Stmt::VarDecl(loop.counter, Expr::makeExpr<Expr::Identifier>(loop.sourcePos, "u16")) },
			false, std::make_optional<Expr::UniquePtr>(std::move(loop.initializer))
		);
		Expr::UniquePtr zero = Expr::makeExpr<Expr::Cast>(loop.sourcePos,
			Expr::makeExpr<Expr::Literal>(loop.sourcePos, Token::Literal(u16{ 0 })),
			Expr::makeExpr<Expr::Identifier>(loop.sourcePos, "u16")
		);
		Expr::UniquePtr condition = Expr::makeExpr<Expr::Binary>(loop.sourcePos, 
			Expr::makeExpr<Expr::Identifier>(loop.sourcePos, loop.counter), 
			Token::Type::EQUAL_EQUAL, 
			std::move(zero)
		);
		addStmtToCurrentBlock(std::move(varInitializer));
		currentBlock().splitWith(Expr::Cloner{}.clone(condition));
		auto [trueBranch, falseBranch] = createChildren();

		auto bodyNode = createTrueChild();
		auto lastLoopNode = visitStmts(bodyNode, loop.body);
		cfg.addEdge(lastLoopNode, bodyNode);
		updateCurrentEntry(lastLoopNode);
		currentBlock().splitWith(std::move(condition));
		updateCurrentEntry(createFalseChild());
		updateCurrentEntry(trueBranch);
		/*
	
		*/
	}

	virtual void visit(Stmt::If& ifStmt) override {
		visitConditionals(ifStmt.ifBranch, ifStmt.elseIfBranch, ifStmt.elseBranch);
	}

	size_t visitConditionals(Stmt::Conditional& trueBlock, std::span<Stmt::Conditional> conditionals, Stmt::StmtBody& elseBranch)
	{
		currentBlock().splitWith(std::move(trueBlock.expr));
		auto [trueBranch, falseBranch] = createChildren();
		auto trueLastNode = visitStmts(trueBranch, trueBlock.body);
		size_t falseLastNode;

		if (!conditionals.empty())
		{
			lastEntry.push(falseBranch);
			falseLastNode = visitConditionals(conditionals[0], conditionals.subspan(1), elseBranch);
			lastEntry.pop();
		}
		else
		{
			falseLastNode = visitStmts(falseBranch, elseBranch);
		}
		size_t joinNode = cfg.createNode(Block::defaultBlock());
		cfg.addEdge(trueLastNode, joinNode);
		cfg.addEdge(falseLastNode, joinNode);
		updateCurrentEntry(joinNode);
		return joinNode;
	}

	virtual void visit(Stmt::Assign& assign) override {
		addStmtToCurrentBlock(std::move(assign));
    }
	virtual void visit(Stmt::ExprStmt& exprStmt) override {
		addStmtToCurrentBlock(std::move(exprStmt));
	}
	virtual void visit(Stmt::Return& stmt) override {
		addStmtToCurrentBlock(std::move(stmt));
		returnBlocks.push_back(lastEntry.top());
	}
	virtual void visit(Stmt::VarDef& varDef) override { 
		addStmtToCurrentBlock(std::move(varDef));
	}
	virtual void visit(Stmt::Label& label) override {
		currentBlock().name(label.label);
	}
	virtual void visit(Stmt::Instruction& stmt) override {
		addStmtToCurrentBlock(std::move(stmt));
	}

	virtual void visit(Stmt::NullStmt& nullStmt) override {} //do nothing

	Block& currentBlock()
	{
		return cfg.nodeData(lastEntry.top());
	}

	template<typename T>
	void addStmtToCurrentBlock(T&& stmt)
	{
        currentBlock().body.emplace_back(Stmt::makeStmt<std::remove_cvref_t<T>>(stmt.sourcePos, std::move(stmt)));
	}
	void addStmtToCurrentBlock(Stmt::UniquePtr stmt)
	{
		currentBlock().body.emplace_back(std::move(stmt));
	}

	std::pair<size_t, size_t> createChildren() 
	{
		size_t lhs = cfg.createNode(Block::trueBlock()), 
			   rhs = cfg.createNode(Block::falseBlock());
		cfg.addEdge(lastEntry.top(), lhs);
		cfg.addEdge(lastEntry.top(), rhs);
		return std::make_pair(lhs, rhs);
	}
	
	size_t createChild() {
		auto newNode = cfg.createNode(Block::defaultBlock());
		cfg.addEdge(lastEntry.top(), newNode);
		return newNode;
	}

	size_t createFalseChild() {
		auto newNode = cfg.createNode(Block::falseBlock());
		cfg.addEdge(lastEntry.top(), newNode);
		return newNode;
	}

	size_t createTrueChild() {
		auto newNode = cfg.createNode(Block::trueBlock());
		cfg.addEdge(lastEntry.top(), newNode);
		return newNode;
	}

	void updateCurrentEntry(size_t joinNode) {
		lastEntry.top() = joinNode;
	}
};