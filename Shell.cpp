/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

static std::vector<std::string> split(std::string_view command)
{
    if (command.empty())
        return {};

    std::vector<std::string> result {};
    std::string builder {};
    bool found_whitespace = false;

    // Ignore whitespace characters at the beginning if any.
    size_t i = 0;
    while (i < command.size() && isspace(command[i]))
        i++;

    for (size_t j = i; j < command.size(); j++) {
        auto character = command[j];
        if (isspace(character)) {
            found_whitespace = true;
            continue;
        }

        if (found_whitespace) {
            found_whitespace = false;
            result.push_back(std::move(builder));
            builder = "";
        }
        builder += character;
    }

    if (!builder.empty())
        result.push_back(std::move(builder));

    return result;
}

namespace ratshell {

int Shell::run_command(std::string const& command)
{
    auto args = split(command);
    if (args.empty())
        return 0;

    auto executable = args[0].c_str();
    if (access(executable, X_OK) < 0)
        return -1;

    auto pid = fork();
    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        std::vector<char*> c_strings {};

        for (auto const& str : args) {
            c_strings.push_back(const_cast<char*>(str.c_str()));
        }
        c_strings.push_back(NULL);

        execv(executable, c_strings.data());

        // If we end up here, exec wasn't successful.
        return -1;
    } else {
        auto child_pid = wait(NULL);
        if (child_pid == -1)
            return -1;
    }

    return 0;
}

}