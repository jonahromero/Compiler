#pragma once
#include <string>
#include <array>
#include <variant>

class Token {
public:
	enum class Type : uint8_t {
		IDENT, NUMBER, STRING, OPCODE, REGISTER, FLAG,
		COMMA, COLON, PESO, PERIOD, BACKSLASH, APOSTROPHE,
		PLUS, MINUS, SLASH, STAR, MODULO,
		EQUAL, EQUAL_EQUAL, NOT_EQUAL,
		LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
		AND, OR, BANG, SHIFT_LEFT, SHIFT_RIGHT,
		BIT_OR, BIT_AND, BIT_XOR, BIT_NOT,
		LEFT_PARENTH, RIGHT_PARENTH,
		NEWLINE,
		EOF_
	};
	using Literal = std::variant<std::monostate, std::string, uint16_t>;
	struct SourcePosition { size_t line, pos; };

	Token(Type type, std::string_view lexeme, Literal literal, SourcePosition sourcePosition)
		: type(type), lexeme(lexeme), literal(literal), sourcePosition(sourcePosition) {}

	Type type;
	std::string_view lexeme;
	Literal literal;
	SourcePosition sourcePosition;
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
		"Identifier", "Number", "String", "Opcode", "Register", "Flag",
		"Comma", "Colon", "Peso", "Period", "BackSlash", "Apostrophe",
		"Plus", "Minus", "Slash", "Star", "Modulo",
		"Equal", "Equal Equal", "Not Equal",
		"Less than", "Less than or Equal", "Greater than", "Greater than or Equal",
		"And", "Or", "Not", "Shift Left", "Shift Right",
		"Bitwise Or", "Bitwise And", "Bitwise Xor", "Bitwise Not",
		"Left Parenthesis", "Right Parenthesis",
		"Newline",
		"End of File"
	};
	return converter[static_cast<size_t>(tokenType)];
}