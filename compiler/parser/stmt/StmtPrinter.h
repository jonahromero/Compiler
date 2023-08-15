#pragma once
#include "Stmt.h"
#include "ExprPrinter.h"

namespace Stmt 
{
	class Printer
		: public Stmt::Visitor, public Expr::Printer {
	public:
		void printStmt(Stmt::UniquePtr& stmt) {
			this->Stmt::Visitor::visitPtr(stmt);
		}

		virtual void visit(Stmt::Label& stmt) {
			prettyPrint("Label: {}", stmt.label);
		}
		virtual void visit(Stmt::Instruction& stmt) {
			prettyPrint("Opcode: {}", stmt.opcode);
			printArglist(stmt.argList);
		}

		virtual void visit(Stmt::Function& func) override {
			prettyPrint("{}fn<{{template}}> {}({{args}}): {{body}}", func.isExported ? "exported " : "", func.name);
			indentCallback([&]() {
				printTemplate(func.templateInfo);
				prettyPrint("Function Arguments:");
				indentCallback([&]() {
					for (auto& decl : func.params) {
						prettyPrint("{} : {{type}}", decl.name);
						indentCallback([&]() {
							printExpr(decl.type);
						});
					}
				});
			});
			printStmts(func.body);
			prettyPrint("\n");
		}
		virtual void visit(Stmt::Bin& bin) override {
			prettyPrint("{}bin<{{template}}> {}: {{body}}", bin.isExported ? "exported " : "", bin.name);
			indentCallback([&]() {
				printTemplate(bin.templateInfo);
				for (auto& decl : bin.body) {
					printDecl(decl);
				}
			});
			prettyPrint("\n");
		}
		virtual void visit(Stmt::Module& mod) override {
			prettyPrint("Declaring Module: {}", mod.title);
			prettyPrint("\n");
		}
		virtual void visit(Stmt::Import& imp) override {
			prettyPrint("Importing: {}", imp.file);
			prettyPrint("\n");
		}
		virtual void visit(Stmt::VarDef& varDef) override {
			auto& init = varDef.initializer;
			if (init.has_value()) {
				prettyPrint("{}{{decl}} {{Initializer}}", varDef.isExported ? "exported " : "");
				indentCallback([&]() {
					printDecl(varDef.decl); 
					printExpr(varDef.initializer.value());
				});
			}
			else {
				prettyPrint("{}{{decl}}", varDef.isExported ? "exported " : "");
				indentCallback([&]() {printDecl(varDef.decl); });
			}
		}
		virtual void visit(Stmt::CountLoop& loop) override {
			prettyPrint("count with {} to {{Expr}}", loop.counter);
			indentCallback([&]() {
				printExpr(loop.initializer);
				printStmts(loop.body);
			});
		}
		virtual void visit(Stmt::Assign& assign) override {
			prettyPrint("{{lhs}} = {{rhs}}");
			indentCallback([&]() {
				printExpr(assign.lhs);
				printExpr(assign.rhs);
			});
		}
		virtual void visit(Stmt::ExprStmt& expr) override {
			prettyPrint("{{expr stmt}}");
			indentCallback([&]() {
				printExpr(expr.expr);
			});
		}
		virtual void visit(Stmt::If& ifStmt) override {
			prettyPrint("If {{Expr}}: {{Body}}");
			indentCallback([&]() {
				printExpr(ifStmt.ifBranch.expr);
				printStmts(ifStmt.ifBranch.body);
			});
			for (auto& elif : ifStmt.elseIfBranch) {
				prettyPrint("Else if {{Expr}}: {{Body}}");
				indentCallback([&]() {
					printExpr(elif.expr);
					printStmts(elif.body);
				});
			}
			if (ifStmt.elseBranch.has_value()) {
				prettyPrint("Else: {{Body}}");
				indentCallback([&]() {
					printStmts(ifStmt.elseBranch.value());
				});
			}
		}
		virtual void visit(Stmt::Return& stmt) override {
			prettyPrint("Return {Expr}");
			indentCallback([&]() {
				printExpr(stmt.expr);
			});
		}
		virtual void visit(Stmt::NullStmt& nullStmt) override {} //do nothing
	private:
		void printArglist(Stmt::ArgList& argList) {
			indentCallback([&]() {
				for (auto& expr : argList) {
					printExpr(expr);
				}
			});
		}
		void printStmts(std::vector<Stmt::UniquePtr>& stmts) {
			//indentCallback([&]() {
				for (auto& stmt : stmts) {
					printStmt(stmt);
				}
			//});
		}
		void printTemplate(auto& templateInfo) {
			prettyPrint("Template Arguments:");
			indentCallback([&]() {
				for (auto& decl : templateInfo.params) {
					printDecl(decl);
				}
				if (templateInfo.params.empty()) {
					prettyPrint("None");
				}
			});
		}
		void printDecl(Stmt::VarDecl& decl) {
			prettyPrint("{} : {{type}}", decl.name);
			indentCallback([&]() {
				printExpr(decl.type);
			});
		}
		void printDecl(Stmt::GenericDecl& decl) {
			if (std::holds_alternative<Stmt::VarDecl>(decl)) {
				printDecl(std::get<Stmt::VarDecl>(decl));
			}
			else if (std::holds_alternative<Stmt::TypeDecl>(decl)) {
				prettyPrint("{} : type", std::get<Stmt::TypeDecl>(decl).name);
			}
		}
	};
}