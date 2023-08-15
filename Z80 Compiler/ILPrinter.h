#pragma once

#include "IL.h"
#include "StmtPrinter.h"
#include "spdlog/fmt/fmt.h"
#include <iostream>
#include "StringUtil.h"

class ILPrinter
	: public IL::Visitor, public StmtPrinter {
public:

	void printIL(IL::UniquePtr& expr) {
		this->visitPtr(expr);
	}

	virtual void visit(IL::Binary& expr) {
		prettyPrint("{} {} = {} {} {}", 
			variableToString(expr.dest), typeToString(expr.type), 
			valueToString(expr.lhs), tokenTypeToStr(expr.operand), valueToString(expr.rhs)
		);
	}
	virtual void visit(IL::Unary& expr) {
		prettyPrint("{} {} = {} {}",
			variableToString(expr.dest), typeToString(expr.type),
			tokenTypeToStr(expr.operand), valueToString(expr.src)
	}
	//Primary expressions
	virtual void visit(IL::Function& func) {
		std::string paramString;
		for (auto& param : func.params) {
			paramString.append(fmt::format("{} {},", variableToString(param.variable), typeToString(param.type)));
		}
		if (!paramString.empty()) paramString.pop_back();
		prettyPrint("{} {}() -> {} {{", func.isExported ? "export" : "",
			func.name, paramString, typeToString(func.returnType)
		);
		indentCallback([&]() {
			for (auto& il : func.stmts) {
				printIL(il);
			}
		});
		prettyPrint("}}");
	}
	virtual void visit(IL::Phi& expr) {
		prettyPrint("{}", "phi");
	}
	virtual void visit(IL::Return& expr) {
		prettyPrint("return {} {}", typeToString(expr.type), valueToString(expr.value));
	}
	virtual void visit(IL::Definition& expr) {
		prettyPrint("{} {} = {}", variableToString(expr.dest), typeToString(expr.type), valueToString(expr.src));
	}
	virtual void visit(IL::Instruction& expr) {
		prettyPrint("[Instruction]");
	}
	virtual void visit(IL::Jump& jump) {
		prettyPrint("jump label {}", jump.target.name);
	}
	virtual void visit(IL::FunctionCall& call) {
		prettyPrint("call {}", call.name);
	}
	virtual void visit(IL::Label& label) {
		prettyPrint("Label {}", label.name);
	}
private:
	auto variableToString(IL::Variable const& var) -> std::string {
		return util::strBuilder(var.is_global ? "@" : "#", std::to_string(var.id));
	}
	auto typeToString(IL::Type const& type) -> std::string {
		std::string_view name;
		using enum IL::DataType;
		switch (type.data_type) {
		case i1: name = "i1"; break;
		case i8: name = "i8"; break;
		case i16: name = "i16"; break;
		case str: name = "str"; break;
		}
		return util::strBuilder(name,
			type.elements > 1 ? util::strBuilder('[', type.elements,']') : "", 
			type.pointer ? "*" : "");
	}
	auto valueToString(IL::Value const& value) -> std::string {
		return std::visit([](auto const& arg) {
			using U = std::remove_cvref_t<decltype(arg)>;
			if constexpr (std::is_same_v<U, IL::PC>) {
				return std::string("$");
			}
			else if (std::is_same_v<U, int>) {
				return std::to_string(arg);
			}
			else if (std::is_same_v<U, std::string>) {
				return arg;
			}
			else if (std::is_same_v<U, IL::Variable>) {
				return variableToString(arg);
			}
		}, value);
	}
};