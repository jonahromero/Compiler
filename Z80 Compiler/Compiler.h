#pragma once
#include <cstdint>
#include <string>
#include "Stmt.h"
#include "Parser.h"
#include "Assembler.h"
#include "Operand.h"
#include "TypeSystem.h"
#include "TemplateSystem.h"
#include "CodeModule.h"
#include "IL.h"
#include "Enviroment.h"
#include "VectorUtil.h"

/*Will eventually support lang features*/
/* Error Handling:
	There can be errors while, 
	Lexing: Handled in Compiler
	Parsing: Handled in Compiler
	Converting to Operands: Handled in Compiler
	Assembling Operands: Handled in Compiler
*/
class VariableCreator {
public:
	auto create_global_variable()-> IL::Variable { return IL::Variable(global++, true); }
	auto create_variable()->IL::Variable { return IL::Variable(global++, false); }
	auto create_label()->IL::Label { return IL::Label(label++); }
private:
	size_t global = 0, variable = 0, label = 0;
};

class NestingLocater {
	void enterLoop() { inLoop = true; }
	void exitLoop() { inLoop = false; }
	void enterFunction() { inGlobalScope = true; }
	void exitFunction() { inGlobalScope = false; }
	void enterConditional() { inIfStmt = true; }
	void exitConditional() { inIfStmt = false; }

	bool inGlobalScope = false;
	bool inIfStmt = false;
	bool inLoop = false;
};

class Compiler : 
	public Stmt::VisitorReturner<std::vector<IL::UniquePtr>>,
	public Expr::VisitorReturner<std::vector<IL::UniquePtr>>,
	public VariableCreator
{
public:
	Compiler() = default;
	auto compile(std::string_view input) -> IL::Program;
private:
	TypeSystem typeSystem;
	Enviroment env;

	using ILProduct = std::vector<IL::UniquePtr>;

	ILProduct visitExpr(Expr::UniquePtr& expr) {
		return this->Expr::VisitorReturner<ILProduct>::visitPtr(expr);
	}
	ILProduct visitStmt(Stmt::UniquePtr& stmt) {
		return this->Stmt::VisitorReturner<ILProduct>::visitPtr(stmt);
	}
	void returnForExpr(std::vector<IL::UniquePtr>&& value) {
		Expr::VisitorReturner<std::vector<IL::UniquePtr>>::returnValue(std::move(value));
	}
	void returnForExpr(IL::UniquePtr value = nullptr) {
		if (value == nullptr) {
			//Expr::VisitorReturner<std::vector<IL::UniquePtr>>::returnValue(util::create_vector<IL::UniquePtr>());
		}
		else {
			//Expr::VisitorReturner<std::vector<IL::UniquePtr>>::returnValue(util::create_vector<IL::UniquePtr>(std::move(value)));
		}
	}
	void returnForStmt(std::vector<IL::UniquePtr>&& value) {
		Stmt::VisitorReturner<std::vector<IL::UniquePtr>>::returnValue(std::move(value));
	}
	void returnForStmt(IL::UniquePtr value = nullptr) {
		if (value == nullptr) {
			Stmt::VisitorReturner<std::vector<IL::UniquePtr>>::returnValue(util::create_vector<IL::UniquePtr>());
		}
		else {
			Stmt::VisitorReturner<std::vector<IL::UniquePtr>>::returnValue(util::create_vector<IL::UniquePtr>(std::move(value)));
		}
	}

	bool inGlobalScope = true;
	auto getILDefinition(std::vector<IL::UniquePtr>& il)->IL::Definition&;
	auto createVariable()->IL::Variable;

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

