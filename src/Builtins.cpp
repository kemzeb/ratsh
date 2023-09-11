/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Builtins.h"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

namespace RatShell {

// https://pubs.opengroup.org/onlinepubs/9699919799/utilities/cd.html#tag_20_14
int builtin_cd(std::vector<std::string> const& argv)
{
    /// TODO: A custom argument parser is needed for this utility.

    if (argv.empty() || argv.size() > 2)
        return 1;

    std::string path;

    if (argv.size() == 1) {
        if (auto* home = std::getenv("HOME"); home != nullptr) {
            path = home;
        } else {
            std::cerr << "failed to get $HOME: " << strerror(errno) << "\n";
            return 1;
        }
    } else {
        path = argv.at(1);
    }

    /// FIXME: Implement step 5 (utilizing CDPATH env var).
    /// FIXME: Implement step 7 once an argument parser has been implemented.

    auto const* pwd = std::getenv("PWD");
    if (pwd == nullptr) {
        std::cerr << "failed to get $PWD: " << strerror(errno) << "\n";
        return 1;
    }

    auto using_old_pwd = false;
    if (path == "-") {
        auto const* old_pwd = std::getenv("OLDPWD");
        if (old_pwd == nullptr) {
            std::cerr << "$OLDPWD is not set\n";
            return 1;
        }
        path = old_pwd;
        using_old_pwd = true;
    }

    std::error_code ec;

    path = std::filesystem::canonical(path, ec);
    if (ec) {
        std::cerr << "failed to create canonical path: " << ec.message() << "\n";
        return 1;
    }

    auto new_pwd = path;

    path = std::filesystem::relative(path, pwd, ec);
    if (ec) {
        std::cerr << "failed to create relative path: " << ec.message() << "\n";
        return 1;
    }
    if (chdir(path.c_str()) == -1) {
        std::cerr << path << ": " << strerror(errno) << "\n";
        return 1;
    }
    if (using_old_pwd)
        std::cout << new_pwd << "\n";

    setenv("PWD", new_pwd.c_str(), 1);
    setenv("OLDPWD", pwd, 1);

    return 0;
}

// https://pubs.opengroup.org/onlinepubs/9699919799/utilities/pwd.html
int builtin_pwd(std::vector<std::string> const&)
{
    // TODO: Implement -L and -P options.

    auto const* pwd = std::getenv("PWD");
    if (pwd == nullptr) {
        std::cerr << "failed to get $PWD: " << strerror(errno) << "\n";
        return 1;
    }

    std::error_code ec;
    auto path = std::filesystem::canonical(pwd, ec);
    if (ec) {
        std::cerr << pwd << ": " << ec.message() << "\n";
        return 1;
    }

    std::cout << path.string() << "\n";

    return 0;
}

} // namespace RatShell