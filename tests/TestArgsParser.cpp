/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ArgsParser.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class ArgsParserTest : public ::testing::Test {
protected:
    virtual void TearDown()
    {
        optind = 1;
        optarg = nullptr;
        optopt = 0;
    };
};

TEST_F(ArgsParserTest, AddBoolOptionNormal)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog", "-o" };
    bool enable_mark = false;

    parser.add_option(enable_mark, "enable the Mark of the Outsider", "", 'o');
    parser.parse(argv);

    ASSERT_EQ(true, enable_mark);
}

TEST_F(ArgsParserTest, AddNonExistentBoolOption)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog", "-a" };
    bool three_flag = false;

    parser.add_option(three_flag, "enable Half Life 3 development", "", 't');
    parser.parse(argv);

    ASSERT_EQ(false, three_flag);
}

TEST_F(ArgsParserTest, AddStringOptionArgument)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog", "-f", "json" };
    std::string file_format;

    parser.add_option_argument(file_format, "choose file format (i.e. json, xml)", "", 'f');
    parser.parse(argv);

    ASSERT_EQ("json", file_format);
}