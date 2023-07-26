/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "Value.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace RatShell::AST {

class Node {
public:
    enum class Kind {
        AndOrIf,
        DupRedirection,
        Execute,
        PathRedirection,
        Pipeline,
        SyntaxError,

        // The following are considered "convenience" nodes.
        ConcatenateListToCommand
    };

    virtual ~Node() = default;

    virtual std::shared_ptr<Value> eval() const = 0;
    virtual Kind kind() const = 0;

    virtual bool is_syntax_error() const { return false; }
};

class SyntaxError final : public Node {
public:
    SyntaxError(std::string error_message)
        : m_error_message(std::move(error_message))
    {
    }

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Node::Kind::SyntaxError; }

    virtual bool is_syntax_error() const override { return true; };

    std::string const& error_message() const { return m_error_message; }

private:
    std::string m_error_message;
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

class PathRedirection final : public Node {
public:
    enum class Flags {
        Read,
        ReadWrite,
        Write,
        WriteAppend
    };

    PathRedirection(std::string path, int fd, Flags flag)
        : m_path(std::move(path))
        , m_fd(fd)
        , m_flags(flag)
    {
    }

    virtual ~PathRedirection() = default;

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::PathRedirection; }

    std::string const& path() const { return m_path; }
    int fd() const { return m_fd; }
    Flags flags() const { return m_flags; }

private:
    std::string m_path;
    int m_fd { -1 };
    Flags m_flags;
};

class DupRedirection final : public Node {
public:
    enum class Type {
        Input,
        Output
    };

    DupRedirection(int left_fd, std::optional<int> right_fd, Type type)
        : m_left_fd(left_fd)
        , m_right_fd(right_fd)
        , m_type(type)
    {
    }

    virtual ~DupRedirection() = default;

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::DupRedirection; }

    int left_fd() const { return m_left_fd; }
    std::optional<int> const& right_fd() const { return m_right_fd; }
    Type type() const { return m_type; }

private:
    int m_left_fd { -1 };
    std::optional<int> m_right_fd;
    Type m_type { Type::Input };
};

class Pipeline final : public Node {
public:
    Pipeline(std::shared_ptr<AST::Node> left, std::shared_ptr<AST::Node> right)
        : m_left(std::move(left))
        , m_right(std::move(right))
    {
    }

    virtual ~Pipeline() = default;

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::Pipeline; }

    std::shared_ptr<AST::Node> const& left() { return m_left; }
    std::shared_ptr<AST::Node> const& right() { return m_right; }

private:
    std::shared_ptr<AST::Node> m_left;
    std::shared_ptr<AST::Node> m_right;
};

class ConcatenateListToCommand final : public Node {
public:
    ConcatenateListToCommand(std::vector<std::shared_ptr<AST::Node>> nodes)
        : m_nodes(std::move(nodes))
    {
    }

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::ConcatenateListToCommand; }

    std::vector<std::shared_ptr<AST::Node>> const& nodes() const { return m_nodes; };

private:
    std::vector<std::shared_ptr<AST::Node>> m_nodes;
};

class AndOrIf final : public Node {
public:
    enum class Type {
        AndIf,
        OrIf
    };

    AndOrIf(std::shared_ptr<AST::Node> left, std::shared_ptr<AST::Node> right, Type type)
        : m_left(std::move(left))
        , m_right(std::move(right))
        , m_type(type)
    {
    }

    virtual std::shared_ptr<Value> eval() const override;
    virtual Kind kind() const override { return Kind::AndOrIf; }

    std::shared_ptr<AST::Node> left() const { return m_left; }
    std::shared_ptr<AST::Node> right() const { return m_right; }

private:
    std::shared_ptr<AST::Node> m_left;
    std::shared_ptr<AST::Node> m_right;
    Type m_type;
};

} // namespace RatShell