/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include "Parser.h"
#include <memory>
#include <sys/stat.h>
#include <sys/wait.h>
#include <vector>

namespace RatShell {

int Shell::run_command(std::string_view input)
{
    auto node = parse(input);

    if (!node)
        return 1;

    if (node->kind() == AST::Node::Kind::Execute) {
        auto exec_node = static_cast<AST::Execute&>(*node);
        return execute_process(exec_node.argv());
    }

    return 1;
}

std::shared_ptr<AST::Node> Shell::parse(std::string_view input) const
{
    Parser parser { input };
    return parser.parse();
}

int Shell::execute_process(std::vector<std::string> const& argv)
{
    if (argv.empty())
        return 0;

    auto const* executable_path = argv[0].c_str();

    struct stat sb;
    auto file_exists = stat(executable_path, &sb) == 0 && S_ISREG(sb.st_mode);
    if (!file_exists)
        return 127;

    auto command_is_executable = access(executable_path, X_OK) == 0;
    if (!command_is_executable)
        return 126;

    auto pid = fork();
    if (pid < 0) {
        /// NOTE: The POSIX spec does not mention what exit code to return when fork() fails.
        return 1;
    }

    if (pid == 0) {
        std::vector<char*> c_strings {};

        c_strings.reserve(argv.size());

        for (auto const& str : argv) {
            c_strings.push_back(const_cast<char*>(str.c_str()));
        }
        c_strings.push_back(NULL);

        /// FIXME: It is possible for execv to fail. We need to account for this.
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