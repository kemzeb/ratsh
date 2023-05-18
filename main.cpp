/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include <iostream>
#include <memory>
#include <string>

int main()
{
    auto shell = std::make_unique<ratshell::Shell>();
    std::string input { "" };

    std::cout << "> ";
    while (true) {
        getline(std::cin, input);

        if (input == "exit")
            return 0;

        int code = shell->run_command(input);

        if (code != 0)
            std::cerr << "An error has occurred. Code " << code << "\n";
        std::cout << "> ";
    }

    return 0;
}