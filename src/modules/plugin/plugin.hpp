/*
 * plugin.hpp
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

#pragma once

#include <string>
#include <vector>

class Plugin
{
public:
    Plugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description);
    virtual ~Plugin();
    virtual void Execute(const std::vector<std::string> &args) const = 0;

    const std::string &GetPath() const;
    const std::string &GetVersion() const;
    const std::string &GetAuthor() const;
    const std::string &GetDescription() const;

private:
    std::string path_;
    std::string version_;
    std::string author_;
    std::string description_;
};
