#pragma once

namespace util 
{

	inline bool isWhitespace(char l) {
		return l == ' ' || l == '\t' || l == '\r';
	}
	inline bool isUppercase(char l) {
		return ('A' <= l && l <= 'Z');
	}
	inline bool isLowercase(char l) {
		return ('a' <= l && l <= 'z');
	}
	inline bool isAlpha(char l) {
		return isUppercase(l) || isLowercase(l);
	}
	inline bool isDigit(char l) {
		return ('0' <= l && l <= '9');
	}
	inline bool isBinary(char l) {
		return l == '0' || l == '1';
	}
	inline bool isHexidecimal(char l) {
		return isDigit(l) || ('A' <= l && l <= 'F') || ('a' <= l && l <= 'f');
	}
	inline bool isCommentStart(char l) {
		return l == ';';
	}
	inline bool isIdentifierStart(char l) {
		return isAlpha(l) || l == '_';
	}
	inline bool isIdentifier(char l) {
		return isIdentifierStart(l) || isDigit(l);
	}
	inline bool isCharQuote(char l) {
		return l == '\'';
	}
	inline bool isStringQuote(char l) {
		return l == '\"';
	}
	inline bool isNotNewline(char l) {
		return l != '\n';
	}
	//conversions
	inline uint8_t hexDigitToUInt8(char l) {
		if (isDigit(l)) {
			return l - '0';
		}
		return l - (isLowercase(l) ? 'a':'A') + 10;
	}

	inline uint8_t decDigitToUInt8(char l) {
		return l - '0';
	}

	inline char toEscapedChar(char l) {
		switch (l) {
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\n';
		default: return l;
		}
	}
}