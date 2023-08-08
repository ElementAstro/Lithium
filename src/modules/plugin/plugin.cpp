/*
 * plugin.cpp
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

Date: 2023-8-6

Description: Basic Plugin Definition

**************************************************/

#include "plugin.hpp"

Plugin::Plugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : path_(path), version_(version), author_(author), description_(description) {}

Plugin::~Plugin() {}

const std::string &Plugin::GetPath() const
{
    return path_;
}

const std::string &Plugin::GetVersion() const
{
    return version_;
}

const std::string &Plugin::GetAuthor() const
{
    return author_;
}

const std::string &Plugin::GetDescription() const
{
    return description_;
}
