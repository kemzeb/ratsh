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

std::shared_ptr<Value> Execute::eval() const
{
    auto command = std::make_shared<CommandValue>();
    command->argv = argv();
    return move(command);
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

std::shared_ptr<Value> DuplicateRedirection::eval() const
{
    if (m_right_fd.has_value())
        return std::make_shared<RedirectionValue>(left_fd(), right_fd().value());

    return std::make_shared<RedirectionValue>(left_fd());
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

    return move(command);
}

}