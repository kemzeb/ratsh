/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace RatShell {

struct Value {
    virtual bool is_command() const { return false; }
    virtual bool is_redirection() const { return false; }
};

struct RedirectionValue final : public Value {
    enum class Action {
        Open,
        Close,
        InputDup,
        OutputDup
    };

    struct PathData {
        std::string path;
        int flags { -1 };
    };

    int io_number { -1 };
    Action action { Action::Open };
    std::variant<PathData, int> redir_variant;

    RedirectionValue(int io_number, std::variant<PathData, int>&& variant = -1)
        : io_number(io_number)
        , redir_variant(std::move(variant))
    {
        if (std::holds_alternative<int>(redir_variant)) {
            auto const& new_fd = std::get<int>(redir_variant);
            if (new_fd < 0)
                action = Action::Close;
        }
    }

    RedirectionValue(int io_number, Action action, std::variant<PathData, int>&& variant)
        : io_number(io_number)
        , action(action)
        , redir_variant(std::move(variant))
    {
    }

    virtual bool is_redirection() const override { return true; }
};

struct CommandValue final : public Value {
    std::vector<std::string> argv;
    std::vector<std::shared_ptr<RedirectionValue>> redirections;

    virtual bool is_command() const override { return true; }
};

}; // namespace RatShell