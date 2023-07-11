/*
 * commander.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-6-17

Description: Commander

**************************************************/

#include "commander.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

bool CommandDispatcher::HasHandler(const std::string &name)
{
    return handlers_.find(Djb2Hash(name.c_str())) != handlers_.end();
}

void CommandDispatcher::Dispatch(const std::string &name, const json &data)
{
    auto it = handlers_.find(Djb2Hash(name.c_str()));
    if (it != handlers_.end())
    {
        spdlog::debug("Find command : {}", name);
        it->second(data);
    }
    else
    {
        spdlog::error("Error: Unknown command {}", name);
    }
}

std::size_t CommandDispatcher::Djb2Hash(const char *str)
{
    std::size_t hash = 5381;
    char c;
    while ((c = *str++) != '\0')
    {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}