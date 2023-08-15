#pragma once
#include "StreamViewer.h"
#include "Token.h"

class TokenViewer 
	: public StreamViewer<Token> {
public:
	class ExpectedType
		: public ExpectedError<Token::Type> {
	public:
		using ExpectedError<Token::Type>::ExpectedError;
	};

	using StreamViewer<Token>::StreamViewer;

	bool atEnd() override {
		return peek().type == Token::Type::EOF_;
	}
	bool matchType(std::same_as<Token::Type> auto...targets) {
		if (auto l = peek(); ((l.type == targets) || ...)) {
			advance();
			return true;
		}
		return false;
	}
	void expectType(Token::Type const& expected) {
		if (expected != peek().type) {
			throw ExpectedType(expected, peek().type);
		}
		else {
			advance();
		}
	}
	virtual ~TokenViewer() = default;
};