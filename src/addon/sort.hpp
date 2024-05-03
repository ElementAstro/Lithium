/*
 * sort.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A sandbox for alone componnents, such as executables.

**************************************************/

#ifndef LITHIUM_COMPONENTS_SORT_HPP
#define LITHIUM_COMPONENTS_SORT_HPP

#include <string>
#include <vector>

namespace lithium {
[[nodiscard("result is discarded")]] std::vector<std::string>
resolveDependencies(const std::vector<std::string>& directories);
}

#endif