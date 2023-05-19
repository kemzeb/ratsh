/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {

std::vector<std::string> split(std::string_view command)
{
    if (command.empty())
        return {};

    std::vector<std::string> result {};
    std::string builder {};
    bool found_whitespace = false;

    // Ignore whitespace characters at the beginning if any.
    size_t i = 0;
    while (i < command.size() && (isspace(command[i]) != 0))
        i++;

    for (size_t j = i; j < command.size(); j++) {
        auto character = command[j];
        if (isspace(character) != 0) {
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

}

namespace ratshell {

int Shell::run_command(std::string const& command)
{
    auto args = split(command);
    if (args.empty())
        return 0;

    auto const* executable_path = args[0].c_str();

    struct stat sb;
    auto file_exists = stat(executable_path, &sb) == 0 && S_ISREG(sb.st_mode);
    if (!file_exists)
        return 127;

    auto command_is_executable = access(executable_path, X_OK) == 0;
    if (!command_is_executable)
        return 126;

    auto pid = fork();
    if (pid < 0) {
        // NOTE: The POSIX spec does not mention what exit code to return when fork() fails.
        return 1;
    }

    if (pid == 0) {
        std::vector<char*> c_strings {};

        c_strings.reserve(args.size());
        for (auto const& str : args) {
            c_strings.push_back(const_cast<char*>(str.c_str()));
        }
        c_strings.push_back(NULL);

        // FIXME: It is possible for execv to fail. We need to account for this.
        execv(executable_path, c_strings.data());
    }

    // If we're here, we must be the parent process.
    int status {};
    wait(&status);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 0;
}

}