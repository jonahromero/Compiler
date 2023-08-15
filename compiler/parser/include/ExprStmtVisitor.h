#pragma once
#include "Expr.h"
#include "Stmt.h"

template<typename T>
class ExprStmtVisitor 
	: public Expr::VisitorReturner<T>, public Stmt::VisitorReturner<T> 
{
public:
	virtual ~ExprStmtVisitor() = default;
	// Exprs

	T visitExpr(Expr::UniquePtr& expr) {
		return this->Expr::VisitorReturner<T>::visitChild(expr);
	}
	void returnForExpr(T value) {
		Expr::VisitorReturner<T>::returnValue(std::move(value));
	}
	void returnForExpr() { Expr::VisitorReturner<T>::returnValue(T{}); }

	// Stmts 

	T visitStmt(Stmt::UniquePtr& stmt) {
		return this->Stmt::VisitorReturner<T>::visitChild(stmt);
	}
	void returnForStmt(T value) {
		Stmt::VisitorReturner<T>::returnValue(std::move(value));
	}
	void returnForStmt() { Stmt::VisitorReturner<T>::returnValue(T{}); }
};