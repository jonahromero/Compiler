
#include <gtest/gtest.h>
#include "Lexer.h"

TEST(LexerTest, Hexidecimal) 
{
	Lexer lexer("0x12");
	auto tokens = lexer.generateTokens();
	EXPECT_EQ(tokens.size(), 2);
	ASSERT_EQ(tokens[0].type, Token::Type::NUMBER);
	ASSERT_EQ(tokens[1].type, Token::Type::EOF_);
	ASSERT_TRUE(std::holds_alternative<u16>(tokens[0].literal));
	ASSERT_EQ(std::get<u16>(tokens[0].literal), 0x12);
}

TEST(LexerTest, Binary)
{
	Lexer lexer("0b101");
	auto tokens = lexer.generateTokens();
	EXPECT_EQ(tokens.size(), 2);
	ASSERT_EQ(tokens[0].type, Token::Type::NUMBER);
	ASSERT_EQ(tokens[1].type, Token::Type::EOF_);
	ASSERT_TRUE(std::holds_alternative<u16>(tokens[0].literal));
	ASSERT_EQ(std::get<u16>(tokens[0].literal), 0b101);
}
