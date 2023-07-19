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

int main()
{
    auto shell = std::make_unique<RatShell::Shell>();
    std::string input;

    std::cout << "> ";
    while (true) {
        getline(std::cin, input);
        if (input == "exit")
            return 0;

        if (!std::cin.good())
            std::cerr << "An unknown error has occurred\n";

        input.push_back('\n'); // Add this so that newlines can be lexed.
        auto code = shell->run_single_line(input);

        if (code != 0)
            std::cerr << "An error has occurred. Code " << code << "\n";
        std::cout << "> ";
    }

    return 0;
}