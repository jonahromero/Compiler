#pragma once
#include "Stmt.h"
#include "ExprPrinter.h"

class StmtPrinter
	: public Stmt::Visitor, public ExprPrinter {

	virtual void visit(Stmt::Label& stmt) {
		prettyPrint("Label", stmt.label);
	}
	virtual void visit(Stmt::Instruction& stmt) {
		prettyPrint("Opcode", stmt.opcode);
		printArglist(stmt.argList);
	}
	virtual void visit(Stmt::NullStmt& stmt) {}
private:
	void printArglist(Stmt::ArgList& argList) {
		indentCallback([&]() {
			for (auto& expr : argList) {
				printExpr(expr);
			}
		});
	}
};