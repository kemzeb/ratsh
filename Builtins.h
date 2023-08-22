
/*
 * Copyright (c) 2023, Kemal Zebari <kemalzebra@gmail.com>.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <string>
#include <vector>

namespace RatShell {

int builtin_cd(std::vector<std::string> const& argv);

} // namespace RatShell