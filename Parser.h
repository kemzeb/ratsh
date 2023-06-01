/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AST.h"
#include "Lexer.h"
#include <memory>
#include <string_view>

namespace RatShell {

class Parser {
public:
    explicit Parser(std::string_view input)
        : m_lexer(input)
    {
    }

    std::shared_ptr<AST::Node> parse();

private:
    bool is_eof() { return m_token_index >= m_token_buffer.size() || m_token_buffer[m_token_index].type == Token::Type::Eof; }

    void fill_token_buffer();

    Token const& consume()
    {
        if (is_eof())
            return m_eof_token;
        return m_token_buffer[m_token_index++];
    }

    Token const& peek()
    {
        if (is_eof())
            return m_eof_token;
        return m_token_buffer[m_token_index];
    }

    std::shared_ptr<AST::Node> parse_simple_command();

    Lexer m_lexer;

    std::vector<Token> m_token_buffer;
    size_t m_token_index { 0 };

    Token m_eof_token { Token::eof() };
};

}