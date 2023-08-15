#pragma once
#include <memory>
#include <vector>
#include "Expr.h"
#include "VisitorReturner.h"

namespace Stmt {

	class Stmt {
	public:
		void accept(class Visitor& visitor) {
			doAccept(visitor);
		}
		template<typename> class VisitorReturner;
		template<typename T>
		T accept(VisitorReturner<T>& visitor);
	private:
		virtual void doAccept(Visitor& visitor) = 0;
	};

	using UniquePtr = std::unique_ptr<Stmt>;
	using ArgList = std::vector<Expr::UniquePtr>;

	template<typename T, typename...Args>
	UniquePtr makeStmt(Args&&...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	class Visitor {
	public:
		void visitStmt(UniquePtr& stmt) {
			stmt->accept(*this);
		}
		virtual void visit(class Instruction& stmt) = 0;
		virtual void visit(class Label& stmt) = 0;
		virtual void visit(class NullStmt& stmt) {}; // Null statements do nothing by default
	};

	template<typename ReturnType>
	class VisitorReturner
		: public ::VisitorReturner<ReturnType>,
		private Visitor
	{
	public:
		ReturnType visitAndReturn(UniquePtr& stmt) {
			return stmt->accept(*this);
		}
		virtual void visit(class NullStmt& stmt) override {
			this->returnValue(ReturnType{});
		}
	};

	template<typename T>
	inline T Stmt::accept(VisitorReturner<T>& visitor)
	{
		doAccept(visitor);
		return visitor.flushRetval();
	}

	//Icky macros probably again...Sorry

#define StmtType(Name, Members, Ctor) \
	class Name : public Stmt { \
	public: \
		Name Ctor {}\
		Members; \
		virtual void doAccept(Visitor& visitor) override { \
			visitor.visit(*this); \
		} \
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
}