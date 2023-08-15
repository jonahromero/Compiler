#pragma once
#include "StreamViewer.h"
#include "Token.h"

class TokenViewer 
	: public StreamViewer<Token> {
public:
	using StreamViewer<Token>::StreamViewer;

	bool atEnd() const override {
		return peek().type == Token::Type::EOF_;
	}
	bool matchType(std::same_as<Token::Type> auto...targets) {
		if (auto l = peek(); ((l.type == targets) || ...)) {
			advance();
			return true;
		}
		return false;
	}
	virtual ~TokenViewer() = default;
};