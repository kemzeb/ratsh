/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <string>
#include <vector>

namespace RatShell::AST {

/// FIXME: What common behaviors should all Node subclasses share?
class Node {
public:
    virtual ~Node() = default;

    enum class Kind {
        Execute
    };

    virtual Kind kind() const = 0;
};

class Execute : public Node {
public:
    Execute(std::vector<std::string> argv)
        : m_argv(std::move(argv))
    {
    }

    virtual ~Execute() = default;

    virtual Kind kind() const override { return Kind::Execute; }

    std::vector<std::string> const& argv() { return m_argv; }

private:
    std::vector<std::string> m_argv;
};

}