#pragma once
#include "Stmt.h"

class TemplateReplacer :
	public Stmt::Visitor, public Expr::Visitor {
public:
	TemplateReplacer(std::string_view targetIdent, Token newToken) 
		: targetIdent(targetIdent), newToken(newToken) {}
	void replaceIn(Stmt::UniquePtr& stmt) {
		Stmt::Visitor::visitPtr(stmt);
	}
private:
	std::string_view targetIdent;
	Token newToken;

	virtual void visit(Stmt::Bin& bin) override {}
	virtual void visit(Stmt::Module& mod) override {} //for now this will do nothing
	virtual void visit(Stmt::Import& imp) override {}
	virtual void visit(Stmt::Function& func) override {}
	virtual void visit(Stmt::NullStmt& nullStmt) override {} //do nothing
	virtual void visit(Stmt::Label& label) override {}
	virtual void visit(Expr::Literal& expr) {}
	virtual void visit(Expr::Register& expr) {}
	virtual void visit(Expr::Flag& expr) {}
	virtual void visit(Expr::CurrentPC& expr) {}


	virtual void visit(Stmt::VarDef& varDef) override { 
		varDef.decl;
	}
	virtual void visit(Stmt::CountLoop& loop) override {}
	virtual void visit(Stmt::Assign& assign) override {}
	virtual void visit(Stmt::If& ifStmt) override {}
	virtual void visit(Stmt::ExprStmt& exprStmt) override {}
	virtual void visit(Stmt::Return& stmt) override {}
	virtual void visit(Stmt::Instruction& stmt) override {}


	virtual void visit(Expr::Binary& expr) {}
	virtual void visit(Expr::Unary& expr) {}
	//Primary expressions
	virtual void visit(Expr::Parenthesis& expr) {}
	virtual void visit(Expr::Identifier& expr) {}
	virtual void visit(Expr::FunctionCall& expr) {}
	virtual void visit(Expr::Indexing& expr) {}
	virtual void visit(Expr::MemberAccess& expr) {}

};