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

    ASSERT_TRUE(enable_mark);
}

TEST_F(ArgsParserTest, AddNonExistentBoolOption)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog", "-a" };
    bool three_flag = false;

    parser.add_option(three_flag, "enable Half Life 3 development", "", 't');
    parser.parse(argv);

    ASSERT_FALSE(three_flag);
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

TEST_F(ArgsParserTest, AddStringOperand)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "mk3", "reptile", "warrior" };
    std::string character;
    std::string tower;

    parser.add_operand(character, "choose your character", "character");
    parser.add_operand(tower, "choose your destiny", "tower");

    ASSERT_TRUE(parser.parse(argv));
    ASSERT_EQ("reptile", character);
    ASSERT_EQ("warrior", tower);
}

TEST_F(ArgsParserTest, AddStringOperandMissing)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog" };
    std::string file_path;

    parser.add_operand(file_path, "file path to project directory", "path");

    ASSERT_FALSE(parser.parse(argv));
    ASSERT_EQ("", file_path);
}

TEST_F(ArgsParserTest, AddStringOperandWithOption)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "mk3", "-m", "bank", "reptile", "warrior" };
    std::string character;
    std::string tower;
    std::string map;

    parser.add_option_argument(map, "choose your map", "", 'm');
    parser.add_operand(character, "choose your character", "character");
    parser.add_operand(tower, "choose your destiny", "tower");

    ASSERT_TRUE(parser.parse(argv));
    ASSERT_EQ("bank", map);
    ASSERT_EQ("reptile", character);
    ASSERT_EQ("warrior", tower);
}

TEST_F(ArgsParserTest, AddStringOperandOptional)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog", "/test/path" };
    std::string file_path;

    parser.add_operand(file_path, "file path to project directory", "path", RatShell::ArgsParser::Required::No);

    ASSERT_TRUE(parser.parse(argv));
    ASSERT_EQ("/test/path", file_path);
}

TEST_F(ArgsParserTest, AddStringOperandOptionalMissing)
{
    RatShell::ArgsParser parser;
    std::vector<std::string> argv = { "prog" };
    std::string file_path;

    parser.add_operand(file_path, "file path to project directory", "path", RatShell::ArgsParser::Required::No);

    ASSERT_TRUE(parser.parse(argv));
    ASSERT_EQ("", file_path);
}