/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include "FileDescription.h"
#include "Parser.h"
#include "Value.h"
#include <cstdio>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <variant>
#include <vector>

namespace RatShell {

namespace {

bool resolve_redirections(std::vector<std::shared_ptr<RedirectionValue>> const& redirections, FileDescriptionCollector& fds, SavedFileDescriptions& saved_fds)
{
    std::vector<std::pair<int, int>> dups;

    for (auto const& redir : redirections) {
        auto fd = redir->io_number;
        auto const& redir_variant = redir->redir_variant;

        // Duplicate fd so we may restore it when the parent returns from the fork call.
        auto saved_fd = dup(fd);
        if (saved_fd < 0) {
            perror("internal: Unable to duplicate fd");
            return false;
        }

        saved_fds.add({ .original = fd, .saved = saved_fd });

        // Make sure to close the duplicated fd for the child process before its execution.
        auto flags = fcntl(saved_fd, F_GETFD);
        auto rc = fcntl(saved_fd, F_SETFD, flags | FD_CLOEXEC);
        if (rc < 0) {
            perror("internal: Unable to change fd flags");
            return false;
        }

        if (redir->action == RedirectionValue::Action::Open) {
            auto const& data = std::get<RedirectionValue::PathData>(redir_variant);
            auto path = data.path;
            auto mode = data.flags;

            // Open a file using the given path.
            auto path_fd = open(path.c_str(), mode, 0666);
            if (path_fd < 0) {
                perror("internal: Unable to open for appending");
                return false;
            }

            fds.add(path_fd);
            dups.push_back({ path_fd, fd });
        } else if (redir->action == RedirectionValue::Action::Close) {
            close(fd);
        } else {
            int flags = fcntl(fd, F_GETFL);
            if (flags < 0) {
                perror("fcntl");
                return false;
            }
            if (redir->action == RedirectionValue::Action::OutputDup) {
                if ((flags & O_RDONLY) != 0) {
                    perror("not open for output");
                    return false; // File is not open for input
                }
            }

            auto const& new_fd = std::get<int>(redir_variant);
            dups.push_back({ new_fd, fd });
        }
    }

    for (auto& dup : dups) {
        auto path_fd = dup.first;
        auto fd_to_redirect = dup.second;

        // Redirect fd to path_fd.
        if (dup2(path_fd, fd_to_redirect) < 0) {
            perror("internal: Unable to redirect");
            return false;
        }
    }

    return true;
}

}

int Shell::run_command(std::string_view input)
{
    if (input.length() == 0)
        return 0;

    auto node = parse(input);

    if (!node)
        return 0;

    auto value = node->eval();
    FileDescriptionCollector fds;
    SavedFileDescriptions saved_fds;

    if (value->is_command()) {
        auto cmd = std::static_pointer_cast<CommandValue>(value);
        if (!resolve_redirections(cmd->redirections, fds, saved_fds))
            return 1;
        return execute_process(cmd->argv);
    }

    return 0;
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

        return execv(executable_path, c_strings.data());
    }

    // If we're here, we must be the parent process.
    int status {};
    wait(&status);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 0;
}

}