#pragma once
#include <unordered_set>
#include <queue>
#include "IL.h"
#include "StmtPrinter.h"
#include "spdlog/fmt/fmt.h"
#include <iostream>
#include "StringUtil.h"

namespace IL
{
	class Printer
		: public ::IL::Visitor, public Stmt::Printer 
	{
		using Stmt::Printer::visit;
	public:

		void printIL(UniquePtr& expr) {
			this->::IL::Visitor::visitChild(expr);
		}

		virtual void visit(Binary& expr) override {
			prettyPrint("{} {} = {} {} {}",
				variableToString(expr.dest.variable), ilTypeToString(expr.dest.type),
				valueToString(expr.lhs), tokenTypeToStr(expr.operation), valueToString(expr.rhs)
			);
		}
		virtual void visit(Unary& expr) override {
			prettyPrint("{} {} = {} {}",
				variableToString(expr.dest.variable), ilTypeToString(expr.dest.type),
				tokenTypeToStr(expr.operation), valueToString(expr.src));
		}
		//Primary expressions
		virtual void visit(Function& func) override {
			std::string paramString;
			for (auto& param : func.params) {
				paramString.append(fmt::format("{} {},", variableToString(param.variable), ilTypeToString(param.type)));
			}
			if (!paramString.empty()) paramString.pop_back();
			prettyPrint("{}fn {}({}) -> {} {{", func.isExported ? "export " : "",
				func.name, paramString, ilTypeToString(func.returnType)
			);
			indentCallback([&]() {
				for (auto& stmt : func.body) {
					printIL(stmt);
				}
			});
			prettyPrint("}}");
		}
		virtual void visit(Test& expr) override {
			prettyPrint("test {} [true => {}]", variableToString(expr.var), expr.trueLabel.name);
		}
		virtual void visit(Phi& expr) override {
			std::string branches;
			prettyPrint("{} {} = phi [{}]", variableToString(expr.dest.variable), ilTypeToString(expr.dest.type), branches);
		}
		virtual void visit(Return& expr) override {
			prettyPrint("return {}", valueToString(expr.value));
		}
		virtual void visit(Assignment& expr) override {
			prettyPrint("{} {} = {}", variableToString(expr.dest.variable), ilTypeToString(expr.dest.type), valueToString(expr.src));
		}
		virtual void visit(Instruction& expr) override {
			prettyPrint("[Instruction]");
		}
		virtual void visit(Jump& jump) override {
			prettyPrint("jump label {}", jump.target.name);
		}
		virtual void visit(FunctionCall& call) override {
			prettyPrint("call {}", call.name);
		}
		virtual void visit(Label& label) override {
			prettyPrint("Label {}", label.name);
		}
		virtual void visit(Cast& cast) override {
			prettyPrint("{} = ({}){}", variableToString(cast.dest), ilTypeToString(cast.cast), valueToString(cast.src));
		}
		virtual void visit(Allocate& allocation) override {
			prettyPrint("{} u8* = alloc({})", variableToString(allocation.dest), std::to_string(allocation.size));
		}
		virtual void visit(Deref& deref) override {
			prettyPrint("{} {} = deref({})", variableToString(deref.dest.variable), ilTypeToString(deref.dest.type), variableToString(deref.ptr));
		}
		virtual void visit(MemCopy& copy) override {
			prettyPrint("memcpy({}, {}, {})", variableToString(copy.dest), variableToString(copy.src), std::to_string(copy.length));
		}
	private:

		auto variableToString(Variable const& var) -> std::string {
			return util::strBuilder(var.is_global ? "@" : "#", std::to_string(var.id));
		}

		auto valueToString(Value const& value) -> std::string {
			return std::visit([this](auto const& arg) {
				using U = std::remove_cvref_t<decltype(arg)>;
				if constexpr (std::is_same_v<U, PC>) {
					return std::string("$");
				}
				else if constexpr (std::is_same_v<U, int>) {
					return std::to_string(arg);
				}
				else if constexpr (std::is_same_v<U, std::string>) {
					return arg;
				}
				else if constexpr (std::is_same_v<U, Variable>) {
					return variableToString(arg);
				}
				}, value);
		}
	};
}