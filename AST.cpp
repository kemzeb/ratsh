/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "AST.h"
#include "Value.h"
#include <cassert>
#include <fcntl.h>
#include <memory>

namespace RatShell::AST {

std::shared_ptr<Value> SyntaxError::eval() const
{
    return nullptr;
}

std::shared_ptr<Value> Execute::eval() const
{
    auto command = std::make_shared<CommandValue>();
    command->argv = argv();
    return command;
}

std::shared_ptr<Value> PathRedirection::eval() const
{
    int open_flags = 0;

    switch (flags()) {
    case Flags::Read:
        open_flags |= O_RDONLY;
        break;
    case Flags::ReadWrite:
        open_flags |= O_CREAT | O_RDWR;
        break;
    case Flags::Write:
        open_flags |= O_CREAT | O_WRONLY | O_TRUNC;
        break;
    case Flags::WriteAppend:
        open_flags |= O_CREAT | O_WRONLY | O_APPEND;
        break;
    }

    auto path_data = RedirectionValue::PathData { .path = path(), .flags = open_flags };
    return std::make_shared<RedirectionValue>(fd(), path_data);
}

std::shared_ptr<Value> DupRedirection::eval() const
{
    if (!m_right_fd.has_value())
        return std::make_shared<RedirectionValue>(left_fd());

    int right_fd = m_right_fd.value();
    if (type() == Type::Input)
        return std::make_shared<RedirectionValue>(left_fd(), RedirectionValue::Action::InputDup, right_fd);

    return std::make_shared<RedirectionValue>(left_fd(), RedirectionValue::Action::OutputDup, right_fd);
}

std::shared_ptr<Value> Pipeline::eval() const
{
    auto left = m_left->eval();
    assert(left->is_command());

    auto cmd = std::static_pointer_cast<CommandValue>(left);

    auto right = m_right->eval();
    assert(right->is_command());

    auto right_cmd = std::static_pointer_cast<CommandValue>(right);
    cmd->next_in_pipeline = right_cmd;

    return cmd;
}

std::shared_ptr<Value> CastListToCommand::eval() const
{
    auto command = std::make_shared<CommandValue>();

    for (auto const& node : nodes()) {
        auto value = node->eval();

        if (value->is_command()) {
            auto other_command = static_pointer_cast<CommandValue>(value);
            command->argv = move(other_command->argv);
        }
        if (value->is_redirection()) {
            auto redirection = static_pointer_cast<RedirectionValue>(value);
            command->redirections.push_back(redirection);
        }
    }

    return command;
}

std::shared_ptr<Value> AndOrIf::eval() const
{
    auto and_or = std::make_shared<AndOrListValue>();

    auto left = m_left->eval();
    assert(left->is_command());

    auto cmd = std::static_pointer_cast<CommandValue>(left);
    cmd->op = m_type == Type::AndIf ? CommandValue::WithOp::AndIf : CommandValue::WithOp::OrIf;
    and_or->commands.push_back(cmd);

    auto right = m_right->eval();
    assert(right->is_command() || right->is_and_or_list());

    if (right->is_command()) {
        auto right_cmd = std::static_pointer_cast<CommandValue>(right);
        and_or->commands.push_back(right_cmd);
    } else {
        auto other_and_or = std::static_pointer_cast<AndOrListValue>(right);
        auto& other_cmds = other_and_or->commands;
        and_or->commands.insert(and_or->commands.end(), other_cmds.begin(), other_cmds.end());
    }

    return and_or;
}

} // namespace RatShell::AST