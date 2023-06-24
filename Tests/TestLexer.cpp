#include "../Lexer.h"
#include <gtest/gtest.h>

namespace RatShell {

// Tests that IoNumber tokens are created when such tokens are next to the start of less/great characters.
TEST(Lexer, BatchNextExpectingIoNumbers)
{
    auto lexer = Lexer { "4<" };
    auto batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::IoNumber, batched_tokens[0].type);

    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::Less, batched_tokens[0].type);

    lexer = Lexer { "16>&" };
    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::IoNumber, batched_tokens[0].type);

    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::GreatAnd, batched_tokens[0].type);
}

TEST(Lexer, BatchNextShouldNotCreateIoNumbers)
{
    auto lexer = Lexer { "4.txt<" };
    auto batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::Token, batched_tokens[0].type);
    ASSERT_EQ("4.txt", batched_tokens[0].value);

    lexer = Lexer { "record78.json>&" };
    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::Token, batched_tokens[0].type);
    ASSERT_EQ("record78.json", batched_tokens[0].value);

    lexer = Lexer { "korvax1>>" };
    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::Token, batched_tokens[0].type);
    ASSERT_EQ("korvax1", batched_tokens[0].value);

    lexer = Lexer { "3gek2&" };
    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::Token, batched_tokens[0].type);
    ASSERT_EQ("3gek2", batched_tokens[0].value);

    lexer = Lexer { "30 >" };
    batched_tokens = lexer.batch_next();
    ASSERT_EQ(1, batched_tokens.size());
    ASSERT_EQ(Token::Type::Token, batched_tokens[0].type);
    ASSERT_EQ("30", batched_tokens[0].value);
}

} // namespace RatShell