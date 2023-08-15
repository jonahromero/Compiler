#pragma once
#include <string>
#include <array>
#include <variant>
#include "IntTypes.h"

class Token {
public:
	enum class Type : u8 {
		COUNT, WITH, FROM, IF, ELSE, RETURN, EXPORT, MODULE, IMPORT, FN, BIN, LET, TYPE,
		IDENT, NUMBER, STRING, OPCODE, REGISTER, FLAG,
		COMMA, COLON, PESO, PERIOD, BACKSLASH,
		PLUS, MINUS, SLASH, STAR, MODULO, TYPE_DEREF, ELLIPSES,
		EQUAL, EQUAL_EQUAL, NOT_EQUAL,
		LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
		AND, OR, BANG, SHIFT_LEFT, SHIFT_RIGHT,
		BIT_OR, BIT_AND, BIT_XOR, BIT_NOT,
		LEFT_PARENTH, RIGHT_PARENTH,
		MUT, TRUE, FALSE, 
		/*White space tokens*/
		NEWLINE, INDENT, DEDENT, 
		EOF_
	};
	using Literal = std::variant<std::monostate, std::string, u16>;
	struct SourcePosition { size_t line, pos; };

	Token(Type type, std::string_view lexeme, Literal literal, SourcePosition sourcePos)
		: type(type), lexeme(lexeme), literal(literal), sourcePos(sourcePos) {}

	Type type;
	std::string_view lexeme;
	Literal literal;
	SourcePosition sourcePos;
};

inline auto literalToStr(Token::Literal const& literal) -> std::string {
	return std::visit([](auto&& value) {
		using T = std::remove_cvref_t<decltype(value)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return std::string("Monostate");
		}
		else if constexpr (std::same_as<T, std::string>) {
			return "\"" + value + "\"";
		}
		else {
			return std::to_string(value);
		}
	}, literal);
}

inline auto tokenTypeToStr(Token::Type tokenType) -> std::string_view {
	static constexpr std::array<std::string_view, static_cast<size_t>(Token::Type::EOF_) + 1> converter = {
		"Count", "With", "From", "If", "Else", "Return", "Export", "Module", "Import", "Function", "Bin", "Let", "Type",
		"Identifier", "Number", "String", "Opcode", "Register", "Flag",
		"Comma", "Colon", "Peso", "Period", "BackSlash", 
		"Plus", "Minus", "Slash", "Star", "Modulo", "Type Operator", "Ellipses",
		"Equal", "Equal Equal", "Not Equal",
		"Less than", "Less than or Equal", "Greater than", "Greater than or Equal",
		"And", "Or", "Not", "Shift Left", "Shift Right",
		"Bitwise Or", "Bitwise And", "Bitwise Xor", "Bitwise Not",
		"Left Parenthesis", "Right Parenthesis",
		"Mutable", "True", "False",
		"Newline", "Indent", "Dedent",
		"End of File"
	};
	return converter[static_cast<size_t>(tokenType)];
}