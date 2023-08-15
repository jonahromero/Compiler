#pragma once
#include <string>
#include <variant>
#include "Token.h"
#include "Expr.h"
#include "Visitors.h"
#include <cassert>
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

namespace IL {

	enum DataType {
		i1, i8, i16, str
	};
	struct Type {
		Type(size_t sizeInBytes) : pointer(false) {
			if (sizeInBytes == 0) {
				data_type = DataType::i8;
				elements = 1;
			}
			else {
				data_type = DataType::i8;
				elements = sizeInBytes;
			}
		}
		Type(DataType data_type, size_t elements = 1) 
			: data_type(data_type), elements(elements), pointer(false) {}
		Type& asPointer() {
			assert(("Cannot create pointer of pointer", !pointer));
			pointer = true;
			return *this;
		}
		bool operator==(Type const& other) const noexcept {
			return data_type == other.data_type && pointer == other.pointer && elements == other.elements;
		}
		DataType data_type;
		size_t elements;
		bool pointer;
	};

	struct PC {};
	struct Variable {
		Variable(size_t id, bool is_global = false) 
			: is_global(is_global) {}
		size_t id;
		bool is_global;
	};
	using Value = std::variant<Variable, std::string, int, PC>;

	class IL
		: public visit::VisitableBase<IL,
		struct Function, struct Binary, struct Unary, struct Phi, struct Return, struct Definition,
		struct Instruction, struct Jump, struct FunctionCall, struct Label> 
	{
	};

	template<typename T>
	class VisitorReturner : public IL::VisitorReturnerType<T> {};
	class Visitor : public IL::VisitorType {};

	using UniquePtr = std::unique_ptr<IL>;
	using Program = std::vector<UniquePtr>;

	auto copyProgram(Program const& program) {
		std::vector<UniquePtr> copiedStmts; copiedStmts.reserve(program.size());
		for (auto const& stmt : program) copiedStmts.push_back(stmt->clone());
		return copiedStmts;
	}

	template<typename T, typename...Args>
	UniquePtr makeIL(Args&&...args) {
		return std::make_unique<T>(T{ std::forward<Args>(args)... });
	}

	struct Function : IL::Visitable<Function> {
		struct Param {
			Type type;
			Variable variable;
		};

		Function() = default;
		Function(std::string_view name, std::vector<Param> params, Type returnType, std::vector<UniquePtr> stmts, bool isExported)
			: name(name), params(std::move(params)), returnType(returnType), stmts(std::move(stmts)), isExported(isExported) {}
		auto deepCopy() const { return Function(name, visit::deepCopyList(params), returnType, copyProgram(stmts), isExported); }

		std::string_view name;
		Type returnType;
		std::vector<Param> params;
		std::vector<UniquePtr> stmts;
		bool isExported;
	};

	// assignments
	struct Binary : IL::Visitable<Binary> {
		Binary(Variable dest, Type type, Value lhs, Token::Type operand, Value rhs)
			: dest(dest), type(type), lhs(lhs), rhs(rhs), operand(operand) {}
		auto deepCopy() { return Binary(dest, type, lhs, operand, rhs); }

		Type type;
		Variable dest;
		Value lhs, rhs;
		Token::Type operand;
	};
	struct Unary : IL::Visitable<Unary> {
		Unary(Variable dest, Type type, Value src, Token::Type operand)
			: dest(dest), type(type), src(src), operand(operand) {}
		auto deepCopy() { return Unary(dest, type, src, operand); }

		Type type;
		Variable dest;
		Value src;
		Token::Type operand;
	};

	struct Definition : IL::Visitable<Definition> {
		Definition(Variable dest, Type type, Value src)
			: type(std::move(type)), dest(dest), src(src) {}
		auto deepCopy() { return Definition(dest, type, src); }

		Type type;
		Variable dest;
		Value src;
	};

	// flow control
	struct Label : IL::Visitable<Label> {
		Label(size_t name) 
			: name(name) {}
		auto deepCopy() { return Label(name); }

		size_t name;
	};
	struct Phi : IL::Visitable<Phi> {
		struct Node {
			Value value;
			Label label;
		};
		Phi(Variable dest, Type type, std::vector<Node> nodes)
			: dest(dest), type(type), nodes(std::move(nodes)) {}
		auto deepCopy() { return Phi(dest, type, nodes); }

		Type type;
		Variable dest;
		std::vector<Node> nodes;
	};
	// misc
	struct Return : IL::Visitable<Return> {
		Return(Type type, Value value)
			: type(type), value(value) {}
		auto deepCopy() { return Return(type, value); }

		Type type;
		Value value;
	};

	struct Instruction : IL::Visitable<Instruction> {
		Instruction(::Stmt::Instruction instr) 
			: instr(std::move(instr)) {}
		auto deepCopy() const { return Instruction(instr.deepCopy()); }

		::Stmt::Instruction instr;
	};
	struct FunctionCall : IL::Visitable<FunctionCall> {
		FunctionCall(std::string_view name, std::vector<Value> args)
			: name(name), args(args) {}
		auto deepCopy() { return FunctionCall(name, args); }

		std::string_view name;
		std::vector<Value> args;
	};
	struct Jump : IL::Visitable<Jump> {
		Jump(Label target) : target(target) {}
		auto deepCopy() { return Jump(target); }

		Label target;
	};
}
