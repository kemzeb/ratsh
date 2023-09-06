/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ArgsParser.h"
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace {

class OptionsParser {
public:
    struct Result {
        int index { 0 };
        int opt { -1 };
        std::optional<std::string> arg;
    };

    static Result parse(int argc, char* const argv[], std::string const& optstring)
    {
        /// TODO: Support long names.

        auto opt = getopt(argc, argv, optstring.c_str());
        std::optional<std::string> arg;

        if (optarg != nullptr)
            arg = optarg;

        return Result {
            .index = optind,
            .opt = opt,
            .arg = std::move(arg)
        };
    }
};

} // namespace

namespace RatShell {

void ArgsParser::add_option(bool& value, std::string help, std::string long_name, char short_name)
{
    auto option = Option {
        .help = std::move(help),
        .long_name = std::move(long_name),
        .short_name = short_name,
        .accept_arg = [&value](std::string_view) {
            value = true;
        }
    };

    add_option(std::move(option));
}

void ArgsParser::add_option_argument(std::string& value, std::string help, std::string long_name, char short_name)
{
    auto option = Option {
        .is_optional_argument = true,
        .help = std::move(help),
        .long_name = std::move(long_name),
        .short_name = short_name,
        .accept_arg = [&value](std::string_view arg) {
            value = arg;
        }
    };

    add_option(std::move(option));
}

void ArgsParser::add_option(Option&& option)
{
    if (!option.long_name.empty()) {
        std::cerr << "FIXME: long names not supported yet\n";
        exit_with_err();
    }

    for (auto const& existing_option : m_options) {
        if (option.short_name == existing_option.short_name) {
            std::cerr << "detected duplicate short name: " << option.short_name << "\n";
            exit_with_err();
        }
    }
    m_options.push_back(std::move(option));
}

bool ArgsParser::parse(std::vector<std::string> const& argv)
{
    int argc = (int)argv.size();
    char* c_argv[argc];

    for (size_t i = 0; i < argv.size(); i++)
        c_argv[i] = const_cast<char*>(argv[i].c_str());

    return parse(argc, c_argv);
}

bool ArgsParser::parse(int argc, char* const argv[])
{
    /// TODO: Support operands (aka positional arguments).

    if (argc < 1)
        return true;

    std::string opstring;

    for (auto const& option : m_options) {
        if (option.short_name != 0) {
            opstring += option.short_name;

            if (option.is_optional_argument)
                opstring += ':';
        }
    }

    while (true) {
        auto result = OptionsParser::parse(argc, argv, opstring);
        auto opt = result.opt;

        if (opt == -1)
            break;
        if (opt == '?')
            return false;

        auto it = std::find_if(m_options.begin(), m_options.end(), [opt](Option const& option) {
            return option.short_name == opt;
        });

        if (it == m_options.end()) {
            std::cerr << "we should not reach this\n";
            exit_with_err();
        }

        auto option = *it;
        auto arg = result.arg.value_or("");

        option.accept_arg(arg);
    }

    return true;
}

} // namespace RatShell