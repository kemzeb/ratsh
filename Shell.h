/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AST.h"
#include <memory>
#include <string>

namespace RatShell {

class Shell {
    enum class Error {
        SyntaxError
    };

public:
    int run_command(std::string_view input);
    void print_error(std::string const& message, Error);

private:
    std::shared_ptr<AST::Node> parse(std::string_view) const;

    int execute_process(std::vector<std::string> const& argv);
};

}