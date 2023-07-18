/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Shell.h"
#include "AST.h"
#include "FileDescription.h"
#include "Parser.h"
#include "Value.h"
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <variant>
#include <vector>

namespace RatShell {

namespace {

bool apply_redirections(std::vector<std::shared_ptr<RedirectionValue>> const& redirections, FileDescriptionCollector& fds, SavedFileDescriptions& saved_fds)
{
    std::vector<std::pair<int, int>> dups;
    FileDescriptionCollector fds_to_be_closed;

    for (auto const& redir : redirections) {
        auto fd = redir->io_number;
        auto const& redir_variant = redir->redir_variant;

        // Duplicate fd so we may restore it when the parent returns from the fork call.
        auto saved_fd = dup(fd);
        if (saved_fd < 0) {
            perror("dup");
            return false;
        }

        saved_fds.add({ .original = fd, .saved = saved_fd });

        // Make sure to close the duplicated fd for the child process before its execution.
        auto flags = fcntl(saved_fd, F_GETFD);
        auto rc = fcntl(saved_fd, F_SETFD, flags | FD_CLOEXEC);
        if (rc < 0) {
            perror("fcntl");
            return false;
        }

        if (redir->action == RedirectionValue::Action::Open) {
            auto const& data = std::get<RedirectionValue::PathData>(redir_variant);
            auto path = data.path;
            auto flags = data.flags;

            // Open a file using the given path.
            auto path_fd = open(path.c_str(), flags, 0666);
            if (path_fd < 0) {
                perror("open");
                return false;
            }

            fds.add(path_fd);
            dups.push_back({ path_fd, fd });
        } else if (redir->action == RedirectionValue::Action::Close) {
            fds_to_be_closed.add(fd);
        } else {
            auto const& right_fd = std::get<int>(redir_variant);
            int flags = fcntl(right_fd, F_GETFL);

            if (flags < 0) {
                perror("fcntl");
                return false;
            }

            auto access = flags & O_ACCMODE;

            if (redir->action == RedirectionValue::Action::OutputDup && access == O_RDONLY) {
                perror("not open for output");
                return false; // File is not open for input
            }
            if (redir->action == RedirectionValue::Action::InputDup && access == O_WRONLY) {
                perror("not open for input");
                return false; // File is not open for input
            }

            dups.push_back({ right_fd, fd });
        }
    }

    // Perform redirections.
    for (auto& dup : dups) {
        auto path_fd = dup.first;
        auto fd_to_redirect = dup.second;

        // Redirect fd to path_fd.
        if (dup2(path_fd, fd_to_redirect) < 0) {
            perror("internal: Unable to redirect");
            return false;
        }
    }

    // Close the desired fds.
    fds_to_be_closed.collect();

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

    if (node->is_syntax_error()) {
        auto err_node = std::static_pointer_cast<AST::SyntaxError>(node);
        print_error(err_node->error_message(), Error::SyntaxError);
        return 1;
    }

    auto value = node->eval();
    FileDescriptionCollector fds;
    SavedFileDescriptions saved_fds;

    if (value->is_command()) {
        auto cmd = std::static_pointer_cast<CommandValue>(value);
        if (!apply_redirections(cmd->redirections, fds, saved_fds))
            return 1;

        auto pid = fork();
        if (pid < 0) {
            /// NOTE: The POSIX spec does not mention what exit code to return when fork() fails.
            return 1;
        }

        if (pid == 0) {
            fds.collect();
            return execute_process(cmd->argv);
        }

        // If we're here, we must be the parent process.
        int status {};
        wait(&status);
        if (WIFEXITED(status))
            return WEXITSTATUS(status);
    }

    return 0;
}

void Shell::print_error(std::string const& message, Error error)
{
    switch (error) {
    case Error::SyntaxError:
        std::cerr << "ratsh (syntax error): " + message + "\n";
    }
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
    std::vector<char*> c_strings {};

    c_strings.reserve(argv.size());

    for (auto const& str : argv) {
        c_strings.push_back(const_cast<char*>(str.c_str()));
    }
    c_strings.push_back(NULL);

    execv(executable_path, c_strings.data());
    exit(errno == ENOENT ? 127 : 126);
}

}