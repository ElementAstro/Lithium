/*
 * ascom_device.hpp
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

Date: 2023-8-15

Description: ASCOM Basic Device

**************************************************/

#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "Hydrogen/core/device.hpp"
#include "cpp_httplib/httplib.h"

#include <any>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using json = nlohmann::json;

#define API_VERSION 1

class ASCOMDevice : virtual public Device
{
public:
    ASCOMDevice(const std::string &name);
    ~ASCOMDevice();

    void setBasicInfo(const std::string &address, const std::string &device_type, const int &device_number);

    virtual bool connect(const IParams &params) override;
    virtual bool disconnect(const IParams &params) override;
    virtual bool reconnect(const IParams &params) override;

public:
    const std::string action(const std::string &action_name, const std::vector<std::any> &parameters);

    void command_blind(const std::string &command_name, bool raw);

    const bool command_bool(const std::string &command_name, bool raw);

    const std::string command_string(const std::string &command_name, bool raw);

    const bool getConnected();

    const void setConnected(bool connecte_state);

    const std::string getDescription();

    const std::vector<std::string> getDriverInfo();

    const std::string getDriverVersion();

    const int getInterfaceVersion();

    const std::string getName();

    const std::vector<std::string> getSupportedActions();

public:
    const std::string get(const std::string &attribute, const json &data = {}, double tmo = 5.0);

    const std::string put(const std::string &attribute, const json &data = {}, double tmo = 5.0);

    void check_error(const httplib::Result &response);

    json convertAnyToJson(const std::any &data)
    {
        if (data.type() == typeid(int))
        {
            return json(std::any_cast<int>(data));
        }
        else if (data.type() == typeid(double))
        {
            return json(std::any_cast<double>(data));
        }
        else if (data.type() == typeid(std::string))
        {
            return json(std::any_cast<std::string>(data));
        }
        return json();
    }

    std::vector<std::string> splitString(const std::string &str, char delimiter)
    {
        std::vector<std::string> substrings;
        std::stringstream ss(str);
        std::string substring;

        while (std::getline(ss, substring, delimiter))
        {
            substrings.push_back(substring);
        }

        return substrings;
    }

    bool stringToBool(const std::string &str)
    {
        std::string lowercaseStr = str;
        std::transform(lowercaseStr.begin(), lowercaseStr.end(), lowercaseStr.begin(), ::tolower);
        if (lowercaseStr == "true" || lowercaseStr == "1" || lowercaseStr == "yes" || lowercaseStr == "on")
        {
            return true;
        }
        return false;
    }

private:
    std::string address;
    std::string device_type;
    int device_number;
    std::string base_url;
    httplib::Client rqs;

    int client_trans_id;
    std::mutex ctid_lock;
    int client_id;
};