#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include <optional>
#include "Expr.h"
#include "Visitors.h"

namespace Stmt 
{
	class Stmt
		: public visit::VisitableBase<Stmt,	struct Instruction, struct Label, struct NullStmt,
		struct Function, struct Bin, struct Module, struct Module, struct Import, struct VarDef, 
		struct CountLoop, struct Assign, struct If, struct Return, struct ExprStmt> {
		public:
			SourcePosition sourcePos;
	};


	template<typename Derived>
	class CloneVisitor : public Expr::CloneVisitor<Derived> {};
	template<typename T>
	class VisitorReturner : public Stmt::VisitorReturnerType<T> {};
	class Visitor : public Stmt::VisitorType {};

	using UniquePtr = std::unique_ptr<Stmt>;
	using ArgList = std::vector<Expr::UniquePtr>;
	using StmtBody = std::vector<::Stmt::UniquePtr>;
	using Program = std::vector<::Stmt::UniquePtr>;

	template<typename T, typename...Args>
	UniquePtr makeStmt(SourcePosition sourcePos, Args&&...args) {
		auto stmt = std::make_unique<T>(T{ std::forward<Args>(args)... });
		stmt->sourcePos = sourcePos;
		return stmt;
	}

// Bad practice....Whatever! 
// (Copies the contents of StmtHelpers.h that need the defitions above to compile.
//  However, would like to have the actual VisitableBase and Visitors in same file.
#include "StmtHelpers.h"

	struct ExprStmt : Stmt::Visitable<ExprStmt> {
		ExprStmt(::Expr::UniquePtr expr) 
			: expr(std::move(expr)) {}

		::Expr::UniquePtr expr;
	};

	struct Function : Stmt::Visitable<Function> 
	{
		Function(TemplateDecl templateInfo, std::string_view name, 
			std::vector<VarDecl> params, std::optional<Expr::UniquePtr> retType, StmtBody body, bool isExported) 
		: templateInfo(std::move(templateInfo)), name(name), params(std::move(params)), 
			retType(std::move(retType)), body(std::move(body)), isExported(isExported) {}

		Function() = default;

		TemplateDecl templateInfo;
		std::string_view name;
		std::vector<VarDecl> params;
		std::optional<Expr::UniquePtr> retType;
		StmtBody body;
		bool isExported;

		bool isTemplate() const { return !templateInfo.params.empty(); }
	};

	struct Bin : Stmt::Visitable<Bin> {
		Bin() = default;
		Bin(TemplateDecl templateInfo, std::string_view name, std::vector<VarDecl> body, bool isExported) 
			: isExported(isExported), name(name), templateInfo(std::move(templateInfo)), body(std::move(body)) {}

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
			: isExported(isExported), decl(std::move(decl)), initializer(std::move(init)) {}

		bool isExported;
		GenericDecl decl;
		std::optional<Expr::UniquePtr> initializer;
	};

	struct CountLoop : Stmt::Visitable<CountLoop> {
		CountLoop(std::string_view counter, Expr::UniquePtr initializer, StmtBody body) 
			: counter(counter), initializer(std::move(initializer)), body(std::move(body)) {}
		CountLoop() = default;

		std::string_view counter;
		Expr::UniquePtr initializer;
		StmtBody body;
	};

	struct Assign : Stmt::Visitable<Assign> {
		Assign(Expr::UniquePtr lhs, Expr::UniquePtr rhs) 
			: lhs(std::move(lhs)), rhs(std::move(rhs)) {}
		Assign() = default;

		Expr::UniquePtr lhs, rhs;
	};
	struct If : Stmt::Visitable<If> {
		If(Conditional ifBranch, std::vector<Conditional> elseIfBranch, StmtBody elseBranch)
			: ifBranch(std::move(ifBranch)), elseIfBranch(std::move(elseIfBranch)), elseBranch(std::move(elseBranch)) {}
		If() = default;

		Conditional ifBranch;
		std::vector<Conditional> elseIfBranch;
		StmtBody elseBranch;
	};

	struct Return : Stmt::Visitable<Return> {
		Return(Expr::UniquePtr expr) : expr(std::move(expr)) {}
		Return() = default;

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

	struct NullStmt : Stmt::Visitable<NullStmt> {
	};
}
