#pragma once
#include <string>
#include <variant>
#include <unordered_map>
#include "CompilerError.h"
#include "Visitors.h"
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
#include "ILType.h"
/*
* @1 = str "asdasd"
* @2 = u8 12
* 
* fn a(u8 #1, i8 #2) -> u8
* types: i1, u8, i8, u16, i16 w/ pointers
* #1 = + u8 #3, 2
* #2 = phi i1 [#1, label #1], [2, label #2], ...
* ret #3
* 
*/

namespace IL 
{
	struct PC {};

	struct Variable 
	{
		Variable(size_t id, bool is_global = false) 
			: id(id), is_global(is_global) {}
		bool operator==(Variable const& other) const { return other.id == id && other.is_global == is_global; }

		size_t id;
		bool is_global;
	};
	using Value = std::variant<Variable, std::string, int, PC>;

	struct Decl 
	{
		Decl(Variable variable, Type type)
			: variable(variable), type(type) {}

		Variable variable;
		Type type;
	};

	class IL
		: public visit::VisitableBase<IL,
		struct Function, struct Binary, struct Unary, struct Phi, struct Return, struct Assignment,
		struct Instruction, struct Jump, struct FunctionCall, struct Label, struct Test, struct Cast, 
		struct Allocate, struct Deref, struct Store, struct MemCopy, struct AddressOf>
	{
	public:
	};

	template<typename T>
	class VisitorReturner : public IL::VisitorReturnerType<T> {};
	class Visitor : public IL::VisitorType {};

	using UniquePtr = std::unique_ptr<IL>;
	using Program = std::vector<UniquePtr>;
	using ILBody = std::vector<UniquePtr>;

	template<typename T, typename...Args>
	UniquePtr makeIL(Args&&...args) {
		return std::make_unique<T>(T{ std::forward<Args>(args)... });
	}

	struct Label : IL::Visitable<Label> 
	{
		Label(size_t name)
			: name(name) {}

		size_t name;
	};

	struct Phi : IL::Visitable<Phi> 
	{
		Phi(Variable dest, Type type, std::vector<Variable> sources)
			: dest(dest, type), sources(std::move(sources)) {}

		Decl dest;
		std::vector<Variable> sources;
	};

	struct Function : IL::Visitable<Function> 
	{
		struct Signature 
		{
			Signature() = default;
			Signature(std::vector<Decl> params, Type returnType)
				: params(std::move(params)), returnType(returnType) {}

			std::vector<Decl> params;
			Type returnType;
		};

		Function(std::string_view name, Signature signature, bool isExported, ILBody body)
			: name(name), signature(std::move(signature)), isExported(isExported), body(std::move(body)) {}
		
		std::string_view name;
		Signature signature;
		bool isExported;
		ILBody body;
	};

	struct Test : IL::Visitable<Test> 
	{
		Test(Variable var, Label trueLabel)
			: var(var), trueLabel(trueLabel) {}

		Variable var;
		Label trueLabel;
	};
	// assignments
	struct Binary : IL::Visitable<Binary> 
	{
		Binary(Variable dest, Type type, Value lhs, Token::Type operation, Value rhs)
			: dest(dest,type), operation(operation), lhs(lhs), rhs(rhs) {}

		Decl dest;
		Token::Type operation;
		Value lhs, rhs;
	};

	struct Unary : IL::Visitable<Unary> 
	{
		Unary(Variable dest, Type type, Token::Type operation, Value src)
			: dest(dest, type), operation(operation), src(src) {}

		Decl dest;
		Token::Type operation;
		Value src;
	};

	struct Assignment : IL::Visitable<Assignment>
	{
		Assignment(Variable dest, Type type, Value src)
			: dest(dest, type), src(src) {}
		
		Decl dest;
		Value src;
	};

	// flow control
	// misc
	struct Return : IL::Visitable<Return> 
	{
		Return(Value value)
			: value(value) {}

		Value value;
	};

	struct Instruction : IL::Visitable<Instruction> 
	{
		Instruction(::Stmt::Instruction instr) 
			: instr(std::move(instr)) {}

		::Stmt::Instruction instr;
	};

	struct FunctionCall : IL::Visitable<FunctionCall> 
	{
		using Callable = std::variant<std::string_view, Variable>;
		FunctionCall(Decl dest, Callable function, std::vector<Value> args)
			: dest(dest), function(function), args(args) {}

		Decl dest;
		Callable function;
		std::vector<Value> args;
	};

	struct Jump : IL::Visitable<Jump> 
	{
		Jump(Label target) : target(target) {}

		Label target;
	};

	struct Cast : IL::Visitable<Cast>
	{
		Cast(Variable dest, Type cast, Variable src)
			: dest(dest), src(src), cast(cast) {}

		Variable dest, src;
		Type cast;
	};

	struct Allocate : IL::Visitable<Allocate>
	{
		Allocate(Variable dest, size_t size)
			: dest(dest), size(size) {}

		Variable dest;
		size_t size;
	};

	struct AddressOf : IL::Visitable<AddressOf>
	{
		AddressOf(Variable ptr, Variable target) 
			: ptr(ptr), target(target) {}

		Variable ptr, target;
	};

	struct Deref : IL::Visitable<Deref> 
	{
		Deref(Variable dest, Type type, Variable ptr) 
			: dest(dest, type), ptr(ptr) {}
		
		Decl dest;
		Variable ptr;
	};

	struct Store : IL::Visitable<Store>
	{
		Store(Variable ptr, Variable src, Type type)
			: ptr(ptr), src(src, type) {}

		Variable ptr;
		Decl src;
	};

	struct MemCopy : IL::Visitable<MemCopy> 
	{
		MemCopy(Variable dest, Variable src, size_t length) 
			: dest(dest), src(src), length(length) {}

		Variable dest, src;
		size_t length;
	};

}

namespace std 
{
	template<>
	class hash<::IL::Variable> 
	{
	public:
		size_t operator()(IL::Variable const& other) const 
		{
			size_t retval = other.id;
			return retval | ((size_t)other.is_global << ((sizeof(size_t) * 8) - 1));
		}
	};
}