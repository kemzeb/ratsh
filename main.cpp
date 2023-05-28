/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Lexer.h"
#include "Shell.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main()
{
    auto shell = std::make_unique<ratshell::Shell>();
    std::string input;

    std::cout << "> ";
    while (true) {
        getline(std::cin, input);

        if (input == "exit")
            return 0;

        /// FIXME: This follwoing lines is just temporary code to see the lexer output.
        /// We should eventually get rid of this.
        auto lexer = RatShell::Lexer { input };
        auto tokens = std::vector<RatShell::Token> {};
        while (true) {
            auto new_tokens = lexer.batch_next();
            if (new_tokens.empty())
                break;

            tokens.insert(tokens.end(), new_tokens.begin(), new_tokens.end());
        }

        for (auto& token : tokens)
            std::cout << token.type_str() << ", " << token.value << "\n";

        // auto code = shell->run_command(input);

        // if (code != 0)
        //     std::cerr << "An error has occurred. Code " << code << "\n";
        std::cout << "> ";
    }

    return 0;
}