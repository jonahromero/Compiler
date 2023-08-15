#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <stack>
#include "Token.h"
#include "StreamViewer.h"
#include "ExpectationErrors.h"

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

	class UnmatchedNester 
		: public LexicalError {
	public:
		UnmatchedNester(size_t line, size_t position, char foundNester, size_t prevNesterLine, char prevNester)
			: LexicalError(line, position), prevNesterLine(prevNesterLine), prevNester(prevNester), foundNester(foundNester) {}

		size_t prevNesterLine;
		char prevNester, foundNester;
	};

	class ShortEllipses
		: public LexicalError {
	public:
		ShortEllipses(size_t line, size_t pos)
			: LexicalError(line, pos) {}
	};

	class UnexpectedCharacter
		: public LexicalError, public UnexpectedError<char> {
	public:
		UnexpectedCharacter(char l, size_t line, size_t pos)
			: LexicalError(line, pos), UnexpectedError<char>(l) {}
	};

	class ExpectedCharacter
		: public LexicalError, public ExpectedError<char> {
	public:
		ExpectedCharacter(char e, std::optional<char> f, size_t line, size_t pos) 
			: LexicalError(line, pos), ExpectedError<char>(e, f) {}
	};

	class UnterminatedCharacterLiteral
		: public LexicalError {
	public:
		using LexicalError::LexicalError;
	};

	Lexer(std::string_view source)
		: StreamViewer<char>(source.data(), source.size()) {
		indentStack.push(0);
	}

	auto generateTokens()->std::vector<Token>;
private:
	auto currentStringView()->std::string_view;

	void addToken(Token::Type type, Token::Literal literal = Token::Literal());
	void addWhitespaceToken(Token::Type type, Token::Literal literal = Token::Literal());
	void checkIndentation();
	void emptyIndentStackUntil(size_t value);
	void addNestingLevel(char current);
	void removeNestingLevel(char current);
	auto calcSourcePos()-> SourcePosition;
	void addNewline();

	void tryToToken();
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
	void expectedCharacter(char letter);
	void shortEllipses();
	void unterminatedCharacterLiteral();

	std::vector<Token> tokens;
	std::stack<size_t> indentStack;
	size_t line = 1, currentSpaces = 0;
	std::stack<std::pair<char, SourcePosition>> nesting;
	bool atStart = true;
};