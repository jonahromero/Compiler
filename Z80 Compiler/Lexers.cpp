#include "Lexer.h"
#include "CharacterUtil.h"
#include "StringUtil.h"
#include "AsciiTable.h"
#include "ReservedIdentifiers.h"
#include "spdlog\spdlog.h"

using enum Token::Type;

auto Lexer::generateTokens() -> std::vector<Token> {
	reset();
	tokens.clear();
	while (!atEnd()) {
		tryToToken();
	}
	emptyIndentStackUntil(0);
	addToken(EOF_);
	return tokens;
}

auto Lexer::currentStringView() -> std::string_view {
	auto view = currentView();
	return std::string_view{ view.data, view.size };
}

void Lexer::checkIndentation()
{
	if (atStart) {
		if (indentStack.top() < currentSpaces) {
			indentStack.push(currentSpaces);
			addWhitespaceToken(INDENT);
		}
		else if (indentStack.top() > currentSpaces) {
			emptyIndentStackUntil(currentSpaces);
			if (indentStack.top() != currentSpaces) { throw "Invalid Indentation"; }
		}
		atStart = false;
	}
}

void Lexer::emptyIndentStackUntil(size_t value)
{
	while (indentStack.top() != value && indentStack.top() != 0) {
		addWhitespaceToken(DEDENT);
		indentStack.pop();
	}
}

auto Lexer::calcSourcePos() -> Token::SourcePosition
{
	return Token::SourcePosition{line, currentPos()};
}

void Lexer::addNewline()
{
	if (!tokens.empty() && tokens.back().type != NEWLINE) {
		addWhitespaceToken(NEWLINE);
	}
	readjustStart();
	atStart = true;
	currentSpaces = 0;
	line++;
}

void Lexer::addToken(Token::Type type, Token::Literal literal) {
#ifndef NDEBUG
	if (type == NEWLINE || type == INDENT || type == DEDENT) { throw "Use addWhiteSpaceToken() instead"; }
#endif
	checkIndentation();
	tokens.push_back(Token(type, currentStringView(), literal, calcSourcePos()));
	readjustStart();
}

void Lexer::addWhitespaceToken(Token::Type type, Token::Literal literal)
{
	std::string_view lexeme = type == NEWLINE ? "\n" : "";
	tokens.push_back(Token(type, lexeme, literal, calcSourcePos()));
}

void Lexer::tryToToken()
{
	try {
		token();
	}
	catch (UnexpectedCharacter const& err) {
		spdlog::error("[Line: {}] Unknown character encountered: \'{}\'.", err.line, err.unexpected);
	}
	catch (ExpectedCharacter const& err) {
		if (err.found.has_value()) {
			spdlog::error("[Line: {}] Expected character: \'{}\', but found: \'{}\'.", err.line, err.expected, err.found.value());
		}
		else {
			spdlog::error("[Line: {}] Expected character: \'{}\', but reached the end of the file.", err.line, err.expected);
		}
	}
	catch (ShortEllipses const& err) {
		spdlog::error("[Line: {}] Ellipses consist of 3 consecutive periods, but only found 2.", err.line);
	}
	catch (UnterminatedCharacterLiteral const& err) {
		spdlog::error("[Line: {}] Unterminated Character Literal.", err.line);
	}
	catch (IntegralTypeTooSmall const& err) {
		spdlog::error("[Line: {}] Integral type is too large to fit in a 16 bit number.", line);
	}
}

void Lexer::token() {
	char letter = advance();
	switch (letter) {
	case '#': addToken(TYPE_DEREF); return;
	case ',': addToken(COMMA); return;
	case ':': addToken(COLON); return;
	case '\\': addToken(BACKSLASH); return;
	case '\n': addNewline(); return;
	case '\r': match('\n') ? addNewline() : throw "Invalid Carraiage Return found.";
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
	case '.': match('.') ? (match('.') ? addToken(ELLIPSES) : shortEllipses()) : addToken(PERIOD); return;
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
	else if (auto [found, tokenType] = getKeyword(ident); found) addToken(tokenType);
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
		if (!match('\'')) {
			unterminatedCharacterLiteral();
		}
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
	if (!match('\'')) {
		expectedCharacter('\'');
	}
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

void Lexer::shortEllipses() {
	throw ShortEllipses(line, currentPos());
}

void Lexer::expectedCharacter(char expect) {
	std::optional<char> found = std::nullopt;
	if (!atEnd()) found = peek();
	throw ExpectedCharacter(expect, found, line, currentPos());
}

void Lexer::unterminatedCharacterLiteral()
{
	throw UnterminatedCharacterLiteral(line, currentPos());
}

void Lexer::whitespace() {
	consumeWhile([&](char l) {
		if (l == ' ') currentSpaces += 1;
		else if (l == '\t') currentSpaces += 4;
		return l == ' ' || l == '\t';
	});
	readjustStart();
}

void Lexer::comment() {
	consumeWhile(isNotNewline);
	readjustStart();
}