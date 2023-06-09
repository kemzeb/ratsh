/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Lexer.h"
#include <cctype>
#include <string_view>
#include <vector>

namespace {

bool is_operator(std::string_view text)
{
    return RatShell::Token::operator_type_from(text).has_value();
}

bool is_part_of_operator(std::string_view text, char ch)
{
    std::string str = text.data();
    str += ch;

    return is_operator(str);
}

}

namespace RatShell {

std::string_view Token::type_str() const
{
    switch (type) {
    case Type::Eof:
        return "Eof";
    case Type::Token:
        return "Token";
    case Type::AndIf:
        return "AndIf";
    case Type::OrIf:
        return "OrIf";
    case Type::DoubleSemicolon:
        return "DoubleSemicolon";
    case Type::DoubleLessThan:
        return "DoubleLessThan";
    case Type::DoubleGreaterThan:
        return "DoubleGreaterThan";
    case Type::LessAnd:
        return "LessAnd";
    case Type::GreatAnd:
        return "GreatAnd";
    case Type::LessGreat:
        return "LessGreat";
    case Type::DoubleLessThanDash:
        return "DoubleLessThanDash";
    case Type::Clobber:
        return "Clobber";
    case Type::Semicolon:
        return "Semicolon";
    case Token::Type::And:
        return "And";
    case Type::OpenParen:
        return "OpenParen";
    case Type::CloseParen:
        return "CloseParen";
    case Type::Pipe:
        return "Pipe";
    case Type::Great:
        return "Great";
    case Type::Less:
        return "Less";
    case Type::Newline:
        return "Newline";
    case Type::Word:
        return "Word";
    }

    return "Unknown";
}

char Lexer::consume()
{
    if (is_eof())
        return '\0';

    return m_input[m_index++];
}

char Lexer::peek() const
{
    if (is_eof())
        return '\0';

    return m_input[m_index];
}

std::vector<Token> Lexer::batch_next()
{
    while (m_next_state_type != StateType::None) {
        auto result = transition(m_next_state_type);
        m_next_state_type = result.next_state_type;

        if (!result.tokens.empty())
            return result.tokens;
    }

    return {};
}

void Lexer::reset_state()
{
    m_state.buffer.clear();
}

Lexer::TransitionResult Lexer::transition(StateType type)
{
    switch (type) {
    case StateType::None:
        return TransitionResult { {}, StateType::None };
    case StateType::Start:
        return transition_start();
    case StateType::End:
        return transition_end();
    case StateType::Operator:
        return transition_operator();
    case StateType::SingleQuotedString:
        return transition_single_quoted_string();
    case StateType::Comment:
        return transition_comment();
    }

    /// TODO: Provide some form of error-handling if we reach here. We shouldn't return anything.
    return { {}, StateType::None };
}

// https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_03
Lexer::TransitionResult Lexer::transition_start()
{
    // 1. If the end of input is recognized, the current token (if any) shall be delimited.
    if (is_eof()) {
        auto maybe_token = Token::generic_token_from(m_state);
        auto tokens = std::vector<Token> {};

        if (maybe_token.has_value())
            tokens.push_back(maybe_token.value());

        reset_state();

        return TransitionResult {
            .tokens = std::move(tokens),
            .next_state_type = StateType::End
        };
    }

    // 4. If the current character is <backslash>, single-quote, or double-quote and it
    // is not quoted, it shall affect quoting for subsequent characters up to the end of
    // the quoted text.
    /// FIXME: Handle backslashes, double quotes, and make sure the current state is not quoted.
    if (peek_is('\'')) {
        m_state.buffer += consume();
        return TransitionResult {
            .tokens = {},
            .next_state_type = StateType::SingleQuotedString
        };
    }

    // 6. If the current character is not quoted and can be used as the first character
    // of a new operator, the current token (if any) shall be delimited. The current
    // character shall be used as the beginning of the next (operator) token.
    /// FIXME: Check for quoting.
    if (is_part_of_operator("", peek())) {
        auto maybe_token = Token::generic_token_from(m_state);
        auto tokens = std::vector<Token> {};

        if (maybe_token.has_value())
            tokens.push_back(maybe_token.value());

        reset_state();
        m_state.buffer += consume();

        return TransitionResult {
            .tokens = std::move(tokens),
            .next_state_type = StateType::Operator
        };
    }

    // 7. If the current character is an unquoted <blank>, any token containing the
    // previous character is delimited and the current character shall be discarded.
    /// FIXME: Check for quoting.
    /// NOTE: This check also includes newlines. Be careful when introducing new code!
    if (isspace(peek()) != 0) {
        consume();
        auto maybe_token = Token::generic_token_from(m_state);
        auto tokens = std::vector<Token> {};

        if (maybe_token.has_value())
            tokens.push_back(maybe_token.value());

        reset_state();

        return TransitionResult {
            .tokens = std::move(tokens),
            .next_state_type = StateType::Start
        };
    }

    // 9. If the current character is a '#', it and all subsequent characters up to, but
    // excluding, the next <newline> shall be discarded as a comment. The <newline> that
    // ends the line is not considered part of the comment.
    if (peek_is('#')) {
        return TransitionResult {
            .tokens = {},
            .next_state_type = StateType::Comment,
        };
    }

    // 8. If the previous character was part of a word, the current character shall be
    // appended to that word.
    // 10. The current character is used as the start of a new word.
    m_state.buffer += consume();
    return TransitionResult {
        .tokens = {},
        .next_state_type = StateType::Start
    };
}

Lexer::TransitionResult Lexer::transition_end()
{
    return TransitionResult {
        .tokens = { Token::eof() },
        .next_state_type = StateType::None
    };
}

// https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_03
Lexer::TransitionResult Lexer::transition_operator()
{
    if (is_eof()) {
        if (is_operator(m_state.buffer)) {
            auto maybe_token = Token::operator_from(m_state);
            auto tokens = std::vector<Token> {};

            if (maybe_token.has_value())
                tokens.push_back(maybe_token.value());

            reset_state();

            return TransitionResult {
                .tokens = std::move(tokens),
                .next_state_type = StateType::End
            };
        }
    }

    // 2. If the previous character was used as part of an operator and the current
    // character is not quoted and can be used with the previous characters to form an
    // operator, it shall be used as part of that (operator) token.
    /// FIXME: Check for quoting.
    if (is_part_of_operator(m_state.buffer, peek())) {
        m_state.buffer += consume();

        return TransitionResult {
            .tokens = {},
            .next_state_type = StateType::Operator
        };
    }

    // 3. If the previous character was used as part of an operator and the current
    // character cannot be used with the previous characters to form an operator, the
    // operator containing the previous character shall be delimited.
    auto tokens = std::vector<Token> {};
    if (is_operator(m_state.buffer)) {
        auto maybe_token = Token::operator_from(m_state);

        if (maybe_token.has_value())
            tokens.push_back(maybe_token.value());

        reset_state();
    }

    return TransitionResult {
        .tokens = std::move(tokens),
        .next_state_type = StateType::Start
    };
}

Lexer::TransitionResult Lexer::transition_single_quoted_string()
{
    /// FIXME: What should we do if this transition is given EOF as input?

    auto ch = consume();
    m_state.buffer += ch;

    if (ch == '\'') {
        // The token shall not be delimited by the end of the quoted field.
        return TransitionResult {
            .tokens = {},
            .next_state_type = StateType::Start
        };
    }

    return TransitionResult {
        .tokens = {},
        .next_state_type = StateType::SingleQuotedString
    };
}

Lexer::TransitionResult Lexer::transition_comment()
{
    if (is_eof()) {
        return TransitionResult {
            .tokens = {},
            .next_state_type = StateType::End,
        };
    }

    if (consume() == '\n') {
        return TransitionResult {
            .tokens = { Token::newline() },
            .next_state_type = StateType::Start,
        };
    }

    return TransitionResult {
        .tokens = {},
        .next_state_type = StateType::Comment,
    };
}

}