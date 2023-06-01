/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Parser.h"
#include "AST.h"
#include "Lexer.h"
#include <memory>

namespace RatShell {

// https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_10_02
std::shared_ptr<AST::Node> Parser::parse()
{
    fill_token_buffer();

    // 1. [Command Name]
    // When the TOKEN is exactly a reserved word, the token identifier for that reserved
    // word shall result. Otherwise, the token WORD shall be returned. Also, if the
    // parser is in any state where only a reserved word could be the next correct
    // token, proceed as above.
    /// FIXME: Check if a token is a reserved word.
    /// FIXME: Should this be here?
    for (auto& token : m_token_buffer) {
        if (token.type == Token::Type::Token)
            token.type = Token::Type::Word;
    }

    /// FIXME: Implement more grammar rules!
    return parse_simple_command();
}

void Parser::fill_token_buffer()
{
    while (true) {

        auto tokens = m_lexer.batch_next();
        if (tokens.empty())
            break;
        m_token_buffer.insert(m_token_buffer.end(), tokens.begin(), tokens.end());
    }
}

std::shared_ptr<AST::Node> Parser::parse_simple_command()
{
    /// FIXME: Implement cmd_prefix-related grammar rules.
    /// FIXME: Implement redirection.

    std::vector<std::string> argv;
    while (true) {
        if (peek().type == Token::Type::Word) {
            /// FIXME: The spec wants rule 1 applied if we encounter cmd_name-related grammar rules.
            auto token = consume();
            argv.push_back(token.value);
        } else {
            break;
        }
    }

    return std::make_shared<AST::Execute>(argv);
}

}