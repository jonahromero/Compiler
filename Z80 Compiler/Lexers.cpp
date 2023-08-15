#include "Lexer.h"
#include "CharacterUtil.h"
#include "StringUtil.h"
#include "AsciiTable.h"
#include "ReservedIdentifiers.h"

using enum Token::Type;

auto Lexer::generateTokens() -> std::vector<Token> {
	reset();
	tokens.clear();
	while (!atEnd()) {
		token();
	}
	addToken(EOF_);
	return tokens;
}

auto Lexer::currentStringView() -> std::string_view {
	auto view = currentView();
	return std::string_view{ view.data, view.size };
}

void Lexer::addToken(Token::Type type, Token::Literal literal) {
	Token::SourcePosition sourcePosition(line, currentPos());
	tokens.push_back(Token(type, currentStringView(), literal, sourcePosition));
	readjustStart();
}

void Lexer::token() {
	char letter = advance();
	switch (letter) {
	case ',': addToken(COMMA); return;
	case ':': addToken(COLON); return;
	case '.': addToken(PERIOD); return;
	case '\\': addToken(BACKSLASH); return;
	case '\n': addToken(NEWLINE); line++; return;
	case '*': addToken(STAR); return;
	case '+': addToken(PLUS); return;
	case '-': addToken(MINUS); return;
	case '/': addToken(SLASH); return;
	case '^': addToken(BIT_XOR); return;
	case '~': addToken(BIT_NOT); return;
	case '(': addToken(LEFT_PARENTH); return;
	case ')': addToken(RIGHT_PARENTH); return;
	case '=': match('=') ? addToken(EQUAL_EQUAL) : addToken(EQUAL); return;
	case '!': match('=') ? addToken(NOT_EQUAL) : addToken(BANG); return;
	case '<': match('=') ? addToken(LESS_EQUAL) : (match('<') ? addToken(SHIFT_LEFT) : addToken(LESS)); return;
	case '>': match('=') ? addToken(GREATER_EQUAL) : (match('>') ? addToken(SHIFT_RIGHT) : addToken(GREATER)); return;
	case '&': match('&') ? addToken(AND) : addToken(BIT_AND); return;
	case '|': match('|') ? addToken(OR) : addToken(BIT_OR); return;
	case '$': isHexidecimal(peek()) ? hexidecimal() : addToken(PESO); return;
	case '%': isBinary(peek()) ? binary() : addToken(MODULO); return;
	}
	if (isDigit(letter)) decimal();
	else if (isCharQuote(letter)) character();
	else if (isStringQuote(letter)) string();
	else if (isIdentifierStart(letter)) identifier();
	else if (isWhitespace(letter)) whitespace();
	else if (isCommentStart(letter)) comment();
	else unexpectedCharacter(letter);
}

void Lexer::identifier()
{
	consumeWhile(isIdentifier);
	auto ident = currentStringView();
	if (isOpcode(ident)) addToken(OPCODE);
	/*Very Special Case: af' is the only identifier that can end with "\'" */
	else if (isRegisterAF(ident) && match('\'')) addToken(REGISTER);
	else if (isRegister(ident)) addToken(REGISTER);
	else if (isFlag(ident)) addToken(FLAG);
	else addToken(IDENT);
}

//needs to check for the end
void Lexer::character()
{
	if (!atEnd()) {
		char letter = advance();
		if (letter == '\\' && !atEnd()) {
			letter = toEscapedChar(advance());
		}
		expect('\'');
		addToken(NUMBER, toAsciiValue(letter));
	}
	else {
		unterminatedCharacterLiteral();
	}
}

void Lexer::string()
{
	std::string literal;
	bool escaped = false;
	consumeWhile([&](char l) {
		if (l == '\"') {
			return false;
		}
		else if (l == '\\') {
			escaped = true;
			return true;
		}
		else if (escaped) {
			literal.push_back(toEscapedChar(l));
			escaped = false;
			return true;
		}
		else {
			literal.push_back(l);
			return true;
		}
		});
	expect('\"');
	addToken(STRING, literal);
}

void Lexer::decimal()
{
	consumeWhile(isDigit);
	auto literal = decToIntegral<uint16_t>(currentStringView());
	addToken(NUMBER, literal);
}

void Lexer::hexidecimal()
{
	consumeWhile(isHexidecimal);
	auto literal = hexToIntegral<uint16_t>(currentStringView().substr(1));
	addToken(NUMBER, literal);
}

void Lexer::binary()
{
	consumeWhile(isBinary);
	auto literal = binaryToIntegral<uint16_t>(currentStringView().substr(1));
	addToken(NUMBER, literal);
}

void Lexer::unexpectedCharacter(char letter) {
	throw UnexpectedCharacter(letter, line, currentPos());
}

void Lexer::unterminatedCharacterLiteral()
{
	throw UnterminatedCharacterLiteral(line, currentPos());
}

void Lexer::whitespace() {
	consumeWhile(isWhitespace);
	readjustStart();
}

void Lexer::comment() {
	consumeWhile(isNotNewline);
	readjustStart();
}