/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AST.h"
#include "Value.h"
#include <memory>
#include <string>
#include <vector>

namespace RatShell {

class Shell {
public:
    enum class Error {
        General,
        SyntaxError
    };

    int run_single_line(std::string_view input);

    void print_error(std::string const& message, Error);

private:
    std::shared_ptr<AST::Node> parse(std::string_view) const;

    int run_command(std::shared_ptr<CommandValue> const&);
    int run_command(std::vector<std::string> const& argv, std::vector<std::shared_ptr<RedirectionValue>> const& redirections);
    int run_commands(std::vector<std::shared_ptr<CommandValue>> const& commands);

    int execute_process(std::vector<std::string> const& argv);
};

}