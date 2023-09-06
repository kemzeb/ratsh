/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace RatShell {

class ArgsParser {
public:
    struct Option {
        bool is_optional_argument { false };
        std::string help;
        std::string long_name;
        char short_name { 0 };
        std::function<void(std::string_view)> accept_arg;
    };

    void add_option(bool& value, std::string help, std::string long_name, char short_name);
    void add_option_argument(std::string& value, std::string help, std::string long_name, char short_name);
    void add_option(Option&&);

    bool parse(std::vector<std::string> const& argv);
    bool parse(int argc, char* const argv[]);

private:
    void exit_with_err()
    {
        exit(1);
    }

    std::vector<Option> m_options;
};

} // namespace RatShell