/*
 * device_exception.hpp
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

Description: Basic Device Exception Defination

*************************************************/

#pragma once

#include <stdexcept>
#include <string>

class InvalidDeviceType : public std::exception
{
public:
    InvalidDeviceType(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

class InvalidParameters : public std::exception
{
public:
    InvalidParameters(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

class InvalidProperty : public std::exception
{
public:
    InvalidProperty(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

class InvalidReturn : public std::exception
{
public:
    InvalidReturn(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

class DispatchError : public std::exception
{
public:
    DispatchError(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};