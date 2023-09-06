#pragma once
#include "Stmt.h"
#include "StmtCloner.h"
#include "ExprInterpreter.h"
#include "MetaUtil.h"

struct TemplateReplacerBase 
{
public:
	void setupReplacement(std::string_view targetIdent, Expr::UniquePtr&& fillerExpr)
	{
		this->targetIdent = targetIdent;
		this->fillerExpr = std::move(fillerExpr);
	}
	Expr::UniquePtr newExpr() const { return Expr::Cloner{}.clone(fillerExpr); }

	std::string_view targetIdent;
	Expr::UniquePtr fillerExpr;
};

class ExprTemplateReplacer :
	public Expr::Visitor, TemplateReplacerBase
{
public:
	ExprTemplateReplacer(Expr::UniquePtr& templatedExpr)
		: templatedExpr(templatedExpr)
	{}

	void replace(std::string_view targetIdent, Expr::UniquePtr&& fillerExpr)
	{
		setupReplacement(targetIdent, std::move(fillerExpr));
		visitExpr(templatedExpr);
	}
	void replace(std::string_view targetIdent, Expr::UniquePtr const& fillerExpr)
	{
		replace(targetIdent, Expr::Cloner{}.clone(fillerExpr));
	}

private:
	Expr::UniquePtr& templatedExpr;
	Expr::UniquePtr* lastExpr = nullptr;
	void visitExpr(Expr::UniquePtr& ptr) { lastExpr = &ptr; visitChild(ptr); }

	virtual void visit(Expr::Binary& expr) override {
		visitExpr(expr.lhs);
		visitExpr(expr.rhs);
	}
	virtual void visit(Expr::Unary& expr) override {
		visitExpr(expr.expr);
	}
	//Primary expressions

	virtual void visit(Expr::KeyworkFunctionCall& expr) override {
		for (auto& arg : expr.args)
			visitExpr(arg);
	}
	virtual void visit(Expr::FunctionType& expr) override {
		visitExpr(expr.returnType);
		for (auto& paramType : expr.paramTypes)
			visitExpr(paramType);
	}
	virtual void visit(Expr::Cast& expr) override {
		visitExpr(expr.expr);
		visitExpr(expr.type);
	}
	virtual void visit(Expr::ListLiteral& expr) override {
		for (auto& arg : expr.elements)
			visitExpr(arg);
	}
	virtual void visit(Expr::StructLiteral& expr) override {
		for(auto& arg : expr.initializers)
			visitExpr(arg);
	}
	virtual void visit(Expr::Reference& expr) override {
		visitExpr(expr.expr);
	}
	virtual void visit(Expr::Questionable& expr) override {
		visitExpr(expr.expr);
	}
	virtual void visit(Expr::Parenthesis& expr) override {
		visitExpr(expr.expr);
	}
	virtual void visit(Expr::Identifier& expr) override {
		if (targetIdent == expr.ident) {
			*lastExpr = newExpr();
		}
	}
	virtual void visit(Expr::FunctionCall& expr) override {
		visitExpr(expr.lhs);
		for (auto& arg : expr.arguments)
			visitExpr(arg);
	}
	virtual void visit(Expr::TemplateCall& expr) override {
		visitExpr(expr.lhs);
		for (auto& arg : expr.templateArgs)
			visitExpr(arg);
	}
	virtual void visit(Expr::Indexing& expr) override {
		visitExpr(expr.lhs);
		visitExpr(expr.innerExpr);
	}
	virtual void visit(Expr::MemberAccess& expr) override {
		visitExpr(expr.lhs);
	}
	virtual void visit(Expr::Literal& expr) override {}
	virtual void visit(Expr::Register& expr) override {}
	virtual void visit(Expr::Flag& expr) override {}
	virtual void visit(Expr::CurrentPC& expr) override {}
};

template<typename StmtType>
class TemplateReplacer :
	public Stmt::Visitor, public TemplateReplacerBase
{
public:
	TemplateReplacer(StmtType const& stmt) 
		: stmt(Stmt::Cloner().clone(stmt)) {}

	void replace(std::string_view targetIdent, Expr::UniquePtr&& fillerExpr)
	{
		setupReplacement(targetIdent, std::move(fillerExpr));
		visitChild(stmt);
	}
	void replace(std::string_view targetIdent, Expr::UniquePtr const& fillerExpr)
	{
		replace(targetIdent, Expr::Cloner{}.clone(fillerExpr));
	}

	StmtType out() {
		return std::move(stmt);
	}

private:
	StmtType stmt;
	
	void visitExpr(Expr::UniquePtr& expr) 
	{
		ExprTemplateReplacer(expr).replace(targetIdent, fillerExpr);
	}
	virtual void visit(Stmt::Bin& bin) override {
		for (auto& field : bin.body) 
		{
			visitExpr(field.type);
		}
	}
	virtual void visit(Stmt::Function& func) override {
		for (auto& stmt : func.body) 
		{
			visitChild(stmt);
		}
		for (auto& param : func.params) {
			visitExpr(param.type);
		}
		visitExpr(func.retType);
	}
	virtual void visit(Stmt::VarDef& varDef) override
	{
		std::visit([&](auto&& arg) {
			using U = std::remove_cvref_t<decltype(arg)>;
			if constexpr (std::is_same_v<U, Stmt::VarDecl>) {
				visitExpr(arg.type);
			}
			}, varDef.decl);

		if (varDef.initializer.has_value())
		{
			visitExpr(varDef.initializer.value());
		}
	}
	virtual void visit(Stmt::CountLoop& loop) override {
		visitExpr(loop.initializer);
		for (auto& stmt : loop.body) visitStmt(stmt);
	}
	virtual void visit(Stmt::Assign& assign) override {
		visitExpr(assign.lhs);
		visitExpr(assign.rhs);
	}
	virtual void visit(Stmt::If& ifStmt) override {
		visitConditional(ifStmt.ifBranch);
		for (auto& branch : ifStmt.elseIfBranch) visitConditional(branch);
		for (auto& stmt : ifStmt.elseBranch) visitStmt(stmt);
	}
	void visitConditional(Stmt::Conditional& conditional) {
		visitExpr(conditional.expr);
		for (auto& stmt : conditional.body) visitStmt(stmt);
	}
	virtual void visit(Stmt::ExprStmt& exprStmt) override {
		visitExpr(exprStmt.expr);
	}
	virtual void visit(Stmt::Return& stmt) override {
		visitExpr(stmt.expr);
	}

	// Cannot recurse into these, so we do nothing
	virtual void visit(Stmt::Module& mod) override {}
	virtual void visit(Stmt::Import& imp) override {}
	virtual void visit(Stmt::NullStmt& nullStmt) override {}
	virtual void visit(Stmt::Label& label) override {}
	virtual void visit(Stmt::Instruction& stmt) override {}
};