#pragma once
#include "Expr.h"
#include "Stmt.h"

template<typename T>
class ExprStmtVisitor 
	: public Expr::VisitorReturner<T>, public Stmt::VisitorReturner<T> {
public:
	// Exprs

	T visitExpr(Expr::UniquePtr& expr) {
		return this->Expr::VisitorReturner<T>::visitPtr(expr);
	}
	void returnForExpr(T value) {
		Expr::VisitorReturner<T>::returnValue(std::move(value));
	}
	void returnForExpr() { Expr::VisitorReturner<T>::returnValue(T{}); }

	// Stmts 

	T visitStmt(Stmt::UniquePtr& stmt) {
		return this->Stmt::VisitorReturner<T>::visitPtr(stmt);
	}
	void returnForStmt(T value) {
		Stmt::VisitorReturner<T>::returnValue(std::move(value));
	}
	void returnForStmt() { Stmt::VisitorReturner<T>::returnValue(T{}); }
};