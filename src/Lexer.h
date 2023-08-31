/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace RatShell {

enum class StateType {
    None,
    Start,
    End,
    Operator,
    SingleQuotedString,
    IoNumber,
    Comment,
};

struct State {
    std::string buffer {};
    bool is_escaping { false };
};

struct Token {
    enum class Type {
        Eof,
        Token,
        AndIf,
        OrIf,
        DoubleSemicolon,
        DoubleLessThan,
        DoubleGreat,
        LessAnd,
        GreatAnd,
        LessGreat,
        DoubleLessThanDash,
        Clobber,
        Semicolon,
        And,
        Pipe,
        OpenParen,
        CloseParen,
        Great,
        Less,
        IoNumber,
        Newline,

        // The following are utilized during parsing.
        Word,
    };

    Type type;
    std::string value;

    static std::optional<Token> generic_token_from(State const& state)
    {
        if (state.buffer.empty())
            return {};

        auto token = Token {
            .type = Type::Token,
            .value = state.buffer
        };

        return token;
    }

    // https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_10_02
    //
    // https://www.gnu.org/software/bash/manual/html_node/Definitions.html
    static std::optional<Token::Type> operator_type_from(std::string_view text)
    {
        if (text == "&&")
            return Token::Type::AndIf;
        if (text == "||")
            return Token::Type::OrIf;
        if (text == ";;")
            return Token::Type::DoubleSemicolon;
        if (text == "<<")
            return Token::Type::DoubleLessThan;
        if (text == ">>")
            return Token::Type::DoubleGreat;
        if (text == "<&")
            return Token::Type::LessAnd;
        if (text == ">&")
            return Token::Type::GreatAnd;
        if (text == "<>")
            return Token::Type::LessGreat;
        if (text == "<<-")
            return Token::Type::DoubleLessThanDash;
        if (text == ";")
            return Token::Type::Semicolon;
        if (text == "&")
            return Token::Type::And;
        if (text == "(")
            return Token::Type::OpenParen;
        if (text == ")")
            return Token::Type::CloseParen;
        if (text == "|")
            return Token::Type::Pipe;
        if (text == ">")
            return Token::Type::Great;
        if (text == "<")
            return Token::Type::Less;
        if (text == "\n")
            return Token::Type::Newline;

        return {};
    }

    static std::optional<Token> operator_from(State const& state)
    {
        auto text = state.buffer;
        auto maybe_type = operator_type_from(text);

        if (!maybe_type.has_value())
            return {};

        auto token = Token {
            .type = maybe_type.value(),
            .value = text,
        };

        return token;
    }

    static Token eof()
    {
        return {
            .type = Type::Eof,
            .value = {},
        };
    }

    static Token newline()
    {
        return {
            .type = Type::Newline,
            .value = "\n",
        };
    }

    std::string_view type_str() const;
};

class Lexer {
public:
    Lexer(std::string_view input)
        : m_input(input) {};

    std::vector<Token> batch_next();

    bool is_eof() const { return m_index >= m_input.length(); }

    char consume()
    {
        if (is_eof())
            return '\0';
        return m_input[m_index++];
    }

    char peek() const
    {
        if (is_eof())
            return '\0';
        return m_input[m_index];
    }

    bool peek_is(char expected) const { return peek() == expected; };

    void skip()
    {
        if (is_eof())
            return;
        m_index++;
    }

private:
    struct TransitionResult {
        std::vector<Token> tokens;
        StateType next_state_type { StateType::None };
    };

    TransitionResult transition(StateType type);
    TransitionResult transition_start();
    TransitionResult transition_end();
    TransitionResult transition_operator();
    TransitionResult transition_single_quoted_string();
    TransitionResult transition_io_number();
    TransitionResult transition_comment();
    void reset_state();

    size_t m_index { 0 };
    std::string_view m_input;

    State m_state;
    StateType m_next_state_type { StateType::Start };
};

} // namespace RatShell