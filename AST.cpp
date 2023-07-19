/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "AST.h"
#include "Value.h"
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

std::shared_ptr<Value> Pipe::eval() const
{
    auto cmd_list = std::make_shared<CommandListValue>();

    cmd_list->is_a_pipe_sequence = true;

    auto left = m_left->eval();
    if (left->is_command()) {
        auto cmd = std::static_pointer_cast<CommandValue>(left);
        cmd_list->cmds.push_back(cmd);
    }

    auto right = m_right->eval();
    if (right->is_command()) {
        auto cmd = std::static_pointer_cast<CommandValue>(right);
        cmd_list->cmds.push_back(cmd);
    } else if (right->is_list()) {
        auto other_list = std::static_pointer_cast<CommandListValue>(right);
        cmd_list->cmds.insert(cmd_list->cmds.end(), other_list->cmds.begin(), other_list->cmds.end());
    }

    return cmd_list;
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

}