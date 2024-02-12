/*
 * httplite.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Simple Http Client

**************************************************/

#pragma once

#include <string>
#include <functional>

std::string httpRequest(const std::string &url, const std::string &method, std::function<void(const std::string &)> errorHandler);