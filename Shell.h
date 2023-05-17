/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <string>

namespace ratshell {

class Shell {

public:
    int run_command(std::string const& command);
};

}