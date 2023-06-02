/*
 * task.hpp
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

Date: 2023-6-1

Description: Basic Task Defination

*************************************************/

#pragma once

#include <string>

#include <nlohmann/json.hpp>

class Task:
{
public:
    Task() {};
    ~Task() {};

public:
    virtual void Run() = 0;
    virtual void Stop() = 0;

    std::string GetName()
    {
        return m_name;
    }

    void SetName(std::string name)
    {
        m_name = name;
    }

    std::string GetDescription()
    {
        return m_description;
    }

    void SetDescription(std::string description)
    {
        m_description = description;
    }

    int GetStatus()
    {
        return m_status;
    }
    
    void SetStatus(int status)
    {
        m_status = status;
    }

private:
    std::string m_name;
    std::string m_description;

    int m_status = 0;
};
