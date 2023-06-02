/*
 * device.hpp
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

Description: Basic Device Defination

*************************************************/

#pragma once

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

class Device:
{
    public:

        explicit Device() {}
        ~Device();

        virtual bool Connect(std::string name) = 0;
        virtual bool Disconnect() = 0;
        virtual bool Reconnect() = 0;
        virtual std::vector<std::string> Scan(std::string name,std::string type) = 0;

        virtual bool ConnectServer(std::string host,int port) = 0;
        virtual bool DisconnectServer() = 0;
        virtual bool ReconnectServer() = 0;
        virtual std::vector<std::string> ScanServer(std::string host) = 0;

        nlohmann::json GetParam(std::string name) = 0;
        bool SetParam(nlo)
}