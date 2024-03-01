/*
 * discovery.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: A simple way to discover HTTP server

**************************************************/

#ifndef ATOM_ALPACA_DISCOVERY_HPP
#define ATOM_ALPACA_DISCOVERY_HPP

#include <string>
#include <vector>

[[nodiscard]] std::vector<std::string> search_ipv4(int numquery, int timeout);
[[nodiscard]] std::vector<std::string> search_ipv6(int numquery, int timeout);

#endif