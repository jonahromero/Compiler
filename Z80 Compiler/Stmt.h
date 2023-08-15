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

	auto copyStmtBody(StmtBody const& body) {
		StmtBody result; result.reserve(body.size());
		for (auto& stmt : body) result.push_back(stmt->clone());
		return result;
	}

	template<typename T, typename...Args>
	UniquePtr makeStmt(Token::SourcePosition sourcePos, Args&&...args) {
		auto stmt = std::make_unique<T>(T{ std::forward<Args>(args)... });
		stmt->sourcePos = sourcePos;
		return stmt;
	}

//i would think this is pretty bad practice.
	struct Conditional {
		Conditional() = default;
		Conditional(Expr::UniquePtr expr)
			: expr(std::move(expr)) {}
		Conditional(Expr::UniquePtr expr, StmtBody body)
			: expr(std::move(expr)), body(std::move(body)) {}
		auto deepCopy() const { return Conditional(expr->clone(), copyStmtBody(body)); }

		::Expr::UniquePtr expr;
		StmtBody body;
	};

	// this is a type decl like: "MyType : type"
	struct TypeDecl {
		TypeDecl(std::string_view name)
			: name(name) {}
		auto deepCopy() const { return TypeDecl(name); }

		std::string_view name;
	};

	//declarators: no expressions or anything fancy
	struct VarDecl {
		VarDecl(std::string_view name, ::Expr::UniquePtr type)
			: name(name), type(std::move(type)) {}
		VarDecl deepCopy() const { return VarDecl(name, type->clone()); }
		std::string_view name;
		::Expr::UniquePtr type;
	};

	auto copyVarDeclList(std::vector<VarDecl> const& varDecls) {
		//std::vector<VarDecl> copiedVarDecls; copiedVarDecls.reserve(varDecls.size());
		//for (auto& varDecl : varDecls) copiedVarDecls.push_back(varDecl.deepCopy());
		//return copiedVarDecls;
		return std::vector<VarDecl>{};
	}

	using GenericDecl = std::variant<VarDecl, TypeDecl>;

	auto copyGenericDecl(GenericDecl const& decl) -> GenericDecl {
		if (std::holds_alternative<VarDecl>(decl)) {
			return std::get<VarDecl>(decl).deepCopy();
		}
		else {
			return std::get<TypeDecl>(decl).deepCopy();
		}
	}

	struct TemplateDecl {
		TemplateDecl() = default;
		TemplateDecl(std::vector<GenericDecl> params) : params(std::move(params)) {}

		std::vector<GenericDecl> params;

		auto deepCopy() const {
			std::vector<GenericDecl> copiedParams; copiedParams.reserve(params.size());
			for (auto& param : params) copiedParams.push_back(copyGenericDecl(param));
			return TemplateDecl{ std::move(copiedParams) };
		}
	};

	struct ExprStmt : Stmt::Visitable<ExprStmt> {
		ExprStmt(::Expr::UniquePtr expr) 
			: expr(std::move(expr)) {}
		auto deepCopy() const { return ExprStmt(expr->clone()); }

		::Expr::UniquePtr expr;
	};

	struct Function : Stmt::Visitable<Function> {
		Function(TemplateDecl templateInfo, std::string_view name, std::vector<VarDecl> params, 
			::Expr::UniquePtr retType, StmtBody body, bool isExported) 
		: templateInfo(std::move(templateInfo)), name(name), params(std::move(params)), retType(std::move(retType)),
			body(std::move(body)), isExported(isExported) {}
		Function() = default;

		bool isExported;
		std::string_view name;

		TemplateDecl templateInfo;
		std::vector<VarDecl> params;
		::Expr::UniquePtr retType;
		StmtBody body;

		bool isTemplate() const { return !templateInfo.params.empty(); }
		auto deepCopy() const {
			return Function(
				templateInfo.deepCopy(), name, copyVarDeclList(params), 
				retType->clone(), copyStmtBody(body), isExported
			); 
		}
	};

	struct Bin : Stmt::Visitable<Bin> {
		Bin() = default;
		Bin(TemplateDecl templateInfo, std::string_view name, std::vector<VarDecl> body, bool isExported) 
			: templateInfo(std::move(templateInfo)), name(name), body(std::move(body)), isExported(isExported) {}
		bool isExported;
		std::string_view name;

		TemplateDecl templateInfo;
		std::vector<VarDecl> body;
		bool isTemplate() const { return !templateInfo.params.empty(); }
		auto deepCopy() const { return Bin(templateInfo.deepCopy(), name, copyVarDeclList(body), isExported); }
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
		auto deepCopy() const {
			return VarDef(copyGenericDecl(decl), isExported, 
				initializer.has_value() ? decltype(initializer){initializer.value()->clone()} : std::nullopt
			);
		}

		bool isExported;
		GenericDecl decl;
		std::optional<Expr::UniquePtr> initializer;
	};
	struct CountLoop : Stmt::Visitable<CountLoop> {
		CountLoop(std::string_view counter, Expr::UniquePtr initializer, StmtBody body) 
			: counter(counter), initializer(std::move(initializer)), body(std::move(body)) {}
		CountLoop() = default;
		auto deepCopy() const { return CountLoop(counter, initializer->clone(), copyStmtBody(body)); }

		std::string_view counter;
		Expr::UniquePtr initializer;
		StmtBody body;
	};
	struct Assign : Stmt::Visitable<Assign> {
		Assign(Expr::UniquePtr lhs, Expr::UniquePtr rhs) 
			: lhs(std::move(lhs)), rhs(std::move(rhs)) {}
		Assign() = default;
		auto deepCopy() const { return Assign(lhs->clone(), rhs->clone()); }

		Expr::UniquePtr lhs, rhs;
	};
	struct If : Stmt::Visitable<If> {
		If(Conditional ifBranch, std::vector<Conditional> elseIfBranch, std::optional<StmtBody> elseBranch)
			: ifBranch(std::move(ifBranch)), elseIfBranch(std::move(elseIfBranch)), elseBranch(std::move(elseBranch)) {}
		If() = default;
		auto deepCopy() const {
			std::vector<Conditional> copiedBranches; copiedBranches.reserve(elseIfBranch.size());
			for (auto& branch : elseIfBranch) copiedBranches.push_back(branch.deepCopy());
			return If(
				ifBranch.deepCopy(), 
				std::move(copiedBranches), 
				elseBranch.has_value() ? decltype(elseBranch){copyStmtBody(elseBranch.value())} : std::nullopt
			); 
		}

		Conditional ifBranch;
		std::vector<Conditional> elseIfBranch;
		std::optional<StmtBody> elseBranch;
	};

	struct Return : Stmt::Visitable<Return> {
		Return(Expr::UniquePtr expr) : expr(std::move(expr)) {}
		Return() = default;
		auto deepCopy() const { return Return(expr->clone()); }

		Expr::UniquePtr expr;
	};

	struct Instruction : Stmt::Visitable<Instruction> {
		Instruction(std::string_view opcode, ArgList argList)
			: opcode(opcode), argList(std::move(argList)) {}
		auto deepCopy() const { return Instruction(opcode, Expr::copyExprList(argList)); }

		std::string_view opcode;
		ArgList argList;
	};
	struct Label : Stmt::Visitable<Label> {
		Label(std::string_view label) : label(label) {}
		std::string_view label;
	};
	struct NullStmt : Stmt::Visitable<NullStmt> {};
}
