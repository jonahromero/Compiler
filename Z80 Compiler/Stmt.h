#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include <optional>
#include "Expr.h"
#include "Visitors.h"

namespace Stmt {

	class Stmt
		: public visit::VisitableBase<Stmt,	struct Instruction, struct Label, struct NullStmt,
		struct Function, struct Bin, struct Module, struct Module, struct Import, struct VarDef, 
		struct CountLoop, struct Assign, struct If, struct Return, struct ExprStmt> {
		public:
			Token::SourcePosition sourcePos;
	};

	template<typename T>
	class VisitorReturner : public Stmt::VisitorReturnerType<T> {};
	class Visitor : public Stmt::VisitorType {};

	using UniquePtr = std::unique_ptr<Stmt>;
	using ArgList = std::vector<Expr::UniquePtr>;
	using StmtBody = std::vector<::Stmt::UniquePtr>;
	using Program = std::vector<::Stmt::UniquePtr>;

	template<typename T, typename...Args>
	UniquePtr makeStmt(Token::SourcePosition sourcePos, Args&&...args) {
		auto stmt = std::make_unique<T>(T{ std::forward<Args>(args)... });
		stmt->sourcePos = sourcePos;
		return stmt;
	}

//i would think this is pretty bad practice.
#include "StmtHelpers.h"

	struct ExprStmt : Stmt::Visitable<ExprStmt> {
		ExprStmt(::Expr::UniquePtr expr) 
			: expr(std::move(expr)) {}
		::Expr::UniquePtr expr;
	};

	struct Function : Stmt::Visitable<Function> {

		bool isExported;
		std::string_view name;

		TemplateDecl templateInfo;
		std::vector<VarDecl> params;
		::Expr::UniquePtr retType;
		StmtBody body;

		bool isTemplate() const { return !templateInfo.params.empty(); }
	};

	struct Bin : Stmt::Visitable<Bin> {
		bool isExported;
		std::string_view name;

		TemplateDecl templateInfo;
		std::vector<VarDecl> body;
		bool isTemplate() const { return !templateInfo.params.empty(); }
	};

	struct Module : Stmt::Visitable<Module>	{ 
		Module(std::string_view title) 
			: title(title) {}
		std::string_view title; 
	};
	struct Import : Stmt::Visitable<Import> {
		Import(std::string_view file)
			: file(file) {}
		std::string_view file;
	};

	struct VarDef : Stmt::Visitable<VarDef> {
		VarDef(GenericDecl decl, bool isExported, std::optional<Expr::UniquePtr> init = std::nullopt) 
			: decl(std::move(decl)), isExported(isExported), initializer(std::move(init)) {}
		bool isExported;
		GenericDecl decl;
		std::optional<Expr::UniquePtr> initializer;
	};
	struct CountLoop : Stmt::Visitable<CountLoop> {
		std::string_view counter;
		Expr::UniquePtr initializer;
		StmtBody body;
	};
	struct Assign : Stmt::Visitable<Assign> {
		Assign(Expr::UniquePtr lhs, Expr::UniquePtr rhs) 
			: lhs(std::move(lhs)), rhs(std::move(rhs)) {}
		Expr::UniquePtr lhs, rhs;
	};
	struct If : Stmt::Visitable<If> {
		Conditional ifBranch;
		std::vector<Conditional> elseIfBranch;
		std::optional<StmtBody> elseBranch;
	};

	struct Return : Stmt::Visitable<Return> {
		Expr::UniquePtr expr;
	};

	struct Instruction : Stmt::Visitable<Instruction> {
		Instruction(std::string_view opcode, ArgList argList)
			: opcode(opcode), argList(std::move(argList)) {}
		std::string_view opcode;
		ArgList argList;
	};
	struct Label : Stmt::Visitable<Label> {
		Label(std::string_view label) : label(label) {}
		std::string_view label;
	};
	struct NullStmt : Stmt::Visitable<NullStmt> {};
}
