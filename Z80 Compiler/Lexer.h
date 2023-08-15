#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "Token.h"
#include "StreamViewer.h"

class Lexer
	: public StreamViewer<char> {
public:
	class LexicalError
		: public std::exception {
	public:
		LexicalError(size_t line, size_t position)
			: line(line), position(position) {}
		size_t line, position;
	};

	class UnexpectedCharacter
		: public LexicalError, public UnexpectedError<char> {
	public:
		UnexpectedCharacter(char l, size_t line, size_t pos)
			: LexicalError(line, pos), UnexpectedError<char>(l) {}
	};

	class UnterminatedCharacterLiteral
		: public LexicalError {
	public:
		using LexicalError::LexicalError;
	};

	Lexer(std::string_view source)
		: StreamViewer<char>(source.data(), source.size()) {}

	auto generateTokens()->std::vector<Token>;
private:
	auto currentStringView()->std::string_view;
	void addToken(Token::Type type, Token::Literal literal = Token::Literal());

	void token();
	void identifier();
	void string();
	void character();
	void whitespace();
	void comment();

	void decimal();
	void hexidecimal();
	void binary();

	void unexpectedCharacter(char letter);
	void unterminatedCharacterLiteral();

	std::vector<Token> tokens;
	size_t line = 1;
};