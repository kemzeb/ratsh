/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Parser.h"
#include "AST.h"
#include "Lexer.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <string>

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
    std::vector<std::shared_ptr<AST::Node>> nodes;
    std::vector<std::string> argv;

    /// TODO: Support prefixed redirection operators and also assginment words.

    if (peek().type == Token::Type::Word) {
        /// TODO: Differentiate between cmd_name and cmd_word grammar.
        argv.push_back(consume().value);
    } else {
        return std::make_shared<AST::SyntaxError>("prefixed redirection not supported yet");
    }

    while (true) {
        if (peek().type == Token::Type::Word) {
            argv.push_back(consume().value);
        } else if (auto io_redirect = parse_io_redirect()) {
            if (io_redirect->is_syntax_error())
                return io_redirect;
            nodes.push_back(io_redirect);
        } else {
            break;
        }
    }
    nodes.push_back(std::make_shared<AST::Execute>(argv));

    return std::make_shared<AST::CastListToCommand>(nodes);
}

std::shared_ptr<AST::Node> Parser::parse_io_redirect()
{
    std::optional<int> io_number;

    if (peek().type == Token::Type::IoNumber) {
        io_number = std::stoi(consume().value);
    }

    if (auto io_file = parse_io_file(io_number))
        return io_file;

    return nullptr;
}

std::shared_ptr<AST::Node> Parser::parse_io_file(std::optional<int> io_number)
{
    switch (peek().type) {
    case Token::Type::Less:
    case Token::Type::LessAnd:
    case Token::Type::Great:
    case Token::Type::GreatAnd:
    case Token::Type::DoubleGreat:
    case Token::Type::LessGreat:
    case Token::Type::Clobber:
        break;
    default:
        return nullptr;
    }

    auto io_operator = consume();

    if (peek().type != Token::Type::Word)
        return std::make_shared<AST::SyntaxError>("no file name given for redirection");

    auto filename = consume();

    switch (io_operator.type) {
    case Token::Type::Less:
        return std::make_shared<AST::PathRedirection>(filename.value, io_number.value_or(0), AST::PathRedirection::Flags::Read);
    case Token::Type::Great:
        return std::make_shared<AST::PathRedirection>(filename.value, io_number.value_or(1), AST::PathRedirection::Flags::Write);
    case Token::Type::DoubleGreat:
        return std::make_shared<AST::PathRedirection>(filename.value, io_number.value_or(1), AST::PathRedirection::Flags::WriteAppend);
    case Token::Type::LessGreat:
        return std::make_shared<AST::PathRedirection>(filename.value, io_number.value_or(0), AST::PathRedirection::Flags::ReadWrite);
    case Token::Type::GreatAnd:
    case Token::Type::LessAnd: {
        int left_fd = io_number.value_or(1);
        std::optional<int> right_fd = std::nullopt;
        AST::DupRedirection::Type type = AST::DupRedirection::Type::Output;

        if (io_operator.type == Token::Type::LessAnd) {
            left_fd = io_number.value_or(0);
            type = AST::DupRedirection::Type::Input;
        }

        if (all_of(filename.value.begin(), filename.value.end(), [](unsigned char c) { return std::isdigit(c); }))
            right_fd = std::stoi(filename.value);
        else if (filename.value != "-")
            return std::make_shared<AST::SyntaxError>("dup operator not given a valid word");

        return std::make_shared<AST::DupRedirection>(left_fd, right_fd, type);
    }
    default:
        return nullptr;
    }
}

} // namespace RatShell