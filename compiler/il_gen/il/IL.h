#pragma once
#include <string>
#include <variant>
#include <unordered_map>
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
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
			: is_global(is_global), id(id) {}
		size_t id;
		bool is_global;
	};
	using Value = std::variant<Variable, std::string, int, PC>;

	struct ILDestination {
		Variable dest;
		Type type;
	};

	class IL
		: public visit::VisitableBase<IL,
		struct Function, struct Binary, struct Unary, struct Phi, struct Return, struct Definition,
		struct Instruction, struct Jump, struct FunctionCall, struct Label, struct Test> 
	{
	public:
		virtual ILDestination getFinalDestination() = 0;
	};

#define NO_FINAL_DESTINATION virtual ILDestination getFinalDestination()override {assert(false); return {Variable(11111), Type(0)};}
#define DEFAULT_FINAL_DESTINATION virtual ILDestination getFinalDestination()override {return {this->dest, this->type};}

	template<typename T>
	class VisitorReturner : public IL::VisitorReturnerType<T> {};
	class Visitor : public IL::VisitorType {};

	using UniquePtr = std::unique_ptr<IL>;
	using Program = std::vector<UniquePtr>;

	template<typename T, typename...Args>
	UniquePtr makeIL(Args&&...args) {
		return std::make_unique<T>(T{ std::forward<Args>(args)... });
	}

	using NodePtr = std::shared_ptr<struct Node>;

	struct Label : IL::Visitable<Label> {
		Label(size_t name)
			: name(name) {}
		auto deepCopy() const { return Label(name); }
		NO_FINAL_DESTINATION;

		size_t name;
	};

	struct Phi : IL::Visitable<Phi> {
		struct NodeSource {
			Value value;
			Label label;
		};
		Phi(Variable dest, Type type, std::vector<NodeSource> nodes)
			: dest(dest), type(type), nodes(std::move(nodes)) {}
		auto deepCopy() const { return Phi(dest, type, nodes); }
		DEFAULT_FINAL_DESTINATION;

		Type type;
		Variable dest;
		std::vector<NodeSource> nodes;
	};

	struct Node {
		Node(Label startLabel, std::vector<UniquePtr> stmts, std::vector<std::shared_ptr<Node>> children)
			: startLabel(startLabel), stmts(std::move(stmts)), children(std::move(children)) {}
		Node(Label startLabel) : startLabel(startLabel) {}
		
		Label startLabel;
		std::vector<UniquePtr> stmts;
		std::vector<std::shared_ptr<Node>> children;

		std::vector<std::string_view> usedVariables;
		std::unordered_map<std::string_view, Phi*> phiNodes;

		std::shared_ptr<Node> deepCopy() {
			std::vector<std::shared_ptr<Node>> new_children;
			for (auto& child : children) {
				new_children.push_back(child->deepCopy());
			}
			return std::make_shared<Node>(startLabel, visit::deepCopyList(stmts), std::move(new_children));
		}
	};

	struct Function : IL::Visitable<Function> {
		struct Param {
			Type type;
			Variable variable;
		};

		Function() = default;
		Function(std::string_view name, std::vector<Param> params, Type returnType, std::shared_ptr<Node> entryNode, bool isExported)
			: name(name), params(std::move(params)), returnType(returnType), entryNode(std::move(entryNode)), isExported(isExported) {}
		auto deepCopy() const { return Function(name, visit::deepCopyList(params), returnType, entryNode->deepCopy(), isExported); }
		NO_FINAL_DESTINATION;

		std::string_view name;
		Type returnType;
		std::vector<Param> params;
		std::shared_ptr<Node> entryNode;
		bool isExported;
	};

	struct Test : IL::Visitable<Test> {
		Test(Variable var, Label trueLabel, Label falseLabel)
			: trueLabel(trueLabel), falseLabel(falseLabel), var(var) {}
		auto deepCopy() const { return Test(var, trueLabel, falseLabel); }
		NO_FINAL_DESTINATION;

		Variable var;
		Label trueLabel, falseLabel;
	};
	// assignments
	struct Binary : IL::Visitable<Binary> {
		Binary(Variable dest, Type type, Value lhs, Token::Type operand, Value rhs)
			: dest(dest), type(type), lhs(lhs), rhs(rhs), operand(operand) {}
		auto deepCopy() const { return Binary(dest, type, lhs, operand, rhs); }
		DEFAULT_FINAL_DESTINATION;

		Type type;
		Variable dest;
		Value lhs, rhs;
		Token::Type operand;
	};
	struct Unary : IL::Visitable<Unary> {
		Unary(Variable dest, Type type, Value src, Token::Type operand)
			: dest(dest), type(type), src(src), operand(operand) {}
		auto deepCopy() const { return Unary(dest, type, src, operand); }
		DEFAULT_FINAL_DESTINATION;

		Type type;
		Variable dest;
		Value src;
		Token::Type operand;
	};

	struct Definition : IL::Visitable<Definition> {
		Definition(Variable dest, Type type, Value src)
			: type(type), dest(dest), src(src) {}
		auto deepCopy() const { return Definition(dest, type, src); }
		DEFAULT_FINAL_DESTINATION;

		Type type;
		Variable dest;
		Value src;
	};

	// flow control
	// misc
	struct Return : IL::Visitable<Return> {
		Return(Type type, Value value)
			: type(type), value(value) {}
		auto deepCopy() const { return Return(type, value); }
		NO_FINAL_DESTINATION;

		Type type;
		Value value;
	};

	struct Instruction : IL::Visitable<Instruction> {
		Instruction(::Stmt::Instruction instr) 
			: instr(std::move(instr)) {}
		auto deepCopy() const { return Instruction(instr.deepCopy()); }
		NO_FINAL_DESTINATION;

		::Stmt::Instruction instr;
	};
	struct FunctionCall : IL::Visitable<FunctionCall> {
		FunctionCall(std::string_view name, std::vector<Value> args)
			: name(name), args(args) {}
		auto deepCopy() const { return FunctionCall(name, args); }
		NO_FINAL_DESTINATION;

		std::string_view name;
		std::vector<Value> args;
	};
	struct Jump : IL::Visitable<Jump> {
		Jump(Label target) : target(target) {}
		auto deepCopy() const { return Jump(target); }
		NO_FINAL_DESTINATION;

		Label target;
	};
}
