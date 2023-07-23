/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace RatShell;

int main()
{
    auto shell = std::make_unique<RatShell::Shell>();
    std::string input;

    while (true) {
        std::cout << "> ";
        getline(std::cin, input);
        if (input == "exit")
            return 0;

        if (std::cin.fail()) {
            shell->print_error("unknown error", Shell::Error::General);
            return 1;
        }
        input.push_back('\n'); // Add this so that newlines can be lexed.
        auto code = shell->run_single_line(input);

        if (code != 0)
            shell->print_error("code " + std::to_string(code), Shell::Error::General);
    }

    return 0;
}