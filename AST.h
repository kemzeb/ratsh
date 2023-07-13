/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "Value.h"
#include <memory>
#include <string>
#include <vector>

namespace RatShell::AST {

class Node {
public:
    enum class Kind {
        Execute,
        Redirection,

        // The following are considered "convenience" nodes.
        CastListToCommand
    };

    virtual ~Node() = default;

    virtual std::shared_ptr<Value> eval() const = 0;
    virtual Kind kind() const = 0;
};

class Execute final : public Node {
public:
    Execute(std::vector<std::string> argv)
        : m_argv(std::move(argv))
    {
    }

    virtual ~Execute() = default;

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::Execute; }

    std::vector<std::string> const& argv() const { return m_argv; }

private:
    std::vector<std::string> m_argv;
};

class Redirection final : public Node {
public:
    enum class Flags {
        Write
    };

    Redirection(std::string path, int fd, Flags flag)
        : m_path(std::move(path))
        , m_fd(fd)
        , m_flags(flag)
    {
    }

    virtual ~Redirection() = default;

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::Redirection; }

    std::string const& path() const { return m_path; }
    int fd() const { return m_fd; }
    Flags flags() const { return m_flags; }

private:
    std::string m_path;
    int m_fd { -1 };
    Flags m_flags;
};

class CastListToCommand final : public Node {
public:
    CastListToCommand(std::vector<std::shared_ptr<AST::Node>> nodes)
        : m_nodes(std::move(nodes))
    {
    }

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::CastListToCommand; }

    std::vector<std::shared_ptr<AST::Node>> const& nodes() const { return m_nodes; };

private:
    std::vector<std::shared_ptr<AST::Node>> m_nodes;
};

} // namespace RatShell