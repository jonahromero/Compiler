#pragma once
#include <memory>
#include <vector>
#include "Expr.h"
#include "Visitors.h"

namespace Stmt {

	class Stmt
		: public visit::VisitableBase<Stmt,
		class Instruction, class Label, class NullStmt> {
	};

	template<typename T>
	class VisitorReturner : public Stmt::VisitorReturnerType<T> {};
	class Visitor : public Stmt::VisitorType {};

	using UniquePtr = std::unique_ptr<Stmt>;
	using ArgList = std::vector<Expr::UniquePtr>;

	template<typename T, typename...Args>
	UniquePtr makeStmt(Args&&...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	struct Instruction : Stmt::Visitable<Instruction> {
		std::string_view opcode;
		ArgList argList;
	};
	struct Label : Stmt::Visitable<Label> {
		std::string_view label;
	};
	struct NullStmt : Stmt::Visitable<NullStmt> {};
}

/* Macros if it repetition is bad enough...
//Icky macros probably again...Sorry
#define StmtType(Name, Members, Ctor) \
	class Name : public Visitable<Stmt, Name> { \
	public: \
		Name Ctor {}\
		Members; \
	} \

	StmtType(Instruction, std::string_view opcode; ArgList argList,
		(std::string_view opcode, ArgList argList)
		: opcode(opcode) PREPROCCESOR_COMMA argList(std::move(argList))
	);
	StmtType(Label, std::string_view label,
		(std::string_view label)
		: label(label)
	);
	StmtType(NullStmt, , ());
}*/