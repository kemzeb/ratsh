/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

struct Value {
    virtual bool is_command() const { return false; }
    virtual bool is_redirection() const { return false; }
};

struct RedirectionValue final : public Value {
    std::string path;
    int fd { -1 };
    int flags { -1 };

    RedirectionValue() = delete;
    RedirectionValue(std::string path, int fd, int flags)
        : path(std::move(path))
        , fd(fd)
        , flags(flags)
    {
    }

    virtual bool is_redirection() const override { return true; }
};

struct CommandValue final : public Value {
    std::vector<std::string> argv;
    std::vector<std::shared_ptr<RedirectionValue>> redirections;

    virtual bool is_command() const override { return true; }
};