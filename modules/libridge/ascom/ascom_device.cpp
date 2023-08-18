/*
 * ascom_device.cpp
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

#include "ascom_device.hpp"
#include "ascom_exception.hpp"

#include "loguru/loguru.hpp"

#include <exception>
#include <format>

ASCOMDevice::ASCOMDevice(const std::string &name) : rqs("localhost"), Device(name)
{
    REGISTER_COMMAND_MEMBER("get_name", getName);
    REGISTER_COMMAND_MEMBER("get_connected", getConnected);
    REGISTER_COMMAND_MEMBER("get_description", getDescription);
    REGISTER_COMMAND_MEMBER("get_driverinfo", getDriverInfo);
    REGISTER_COMMAND_MEMBER("get_interfaceversion", getInterfaceVersion);
    REGISTER_COMMAND_MEMBER("get_supportedactions", getSupportedActions);

    insertBoolProperty("connection", false, {}, PossibleValueType::None);
}

ASCOMDevice::~ASCOMDevice()
{
}

void ASCOMDevice::setBasicInfo(const std::string &address, const std::string &device_type, const int &device_number)
{
    this->address = address;
    this->device_number = device_number;
    this->device_type = device_type;
    this->base_url = std::format("http://{}/api/v{}/{}/{}", this->address, API_VERSION, this->device_type, this->device_number);
}

bool ASCOMDevice::connect(const std::string &name)
{
    if (getConnected())
    {
        LOG_F(WARNING, "Connection had already been established , please do not connect again");
        return true;
    }
    setConnected(true);
    if (!getConnected())
    {
        LOG_F(ERROR, "Failed to establish connection with %s", name.c_str());
        return false;
    }
    LOG_F(INFO, "Connected to %s", name.c_str());
    return true;
}

bool ASCOMDevice::disconnect()
{
    if (!getConnected())
    {
        LOG_F(WARNING, "Connection is not established, please do not run disconnect command");
        return true;
    }
    setConnected(false);
    if (getConnected())
    {
        LOG_F(ERROR, "Failed to disconnect with %s", getStringProperty("name")->value.c_str());
        return false;
    }
    return true;
}

bool ASCOMDevice::reconnect()
{
    if (!disconnect())
    {
        LOG_F(ERROR, "Failed to reconnect with %s, falied when trying to disconnect with", getStringProperty("name")->value.c_str());
    }
    if (!connect(getStringProperty("name")->value))
    {
        LOG_F(ERROR, "Failed to reconnect %s, falied when trying to connect to", getStringProperty("name")->value.c_str());
    }
    return true;
}

const std::string ASCOMDevice::action(const std::string &action_name, const std::vector<std::any> &parameters)
{
    json params = json::array();
    for (const auto &item : parameters)
    {
        params.push_back(convertAnyToJson(item));
    }
    return put("action", params);
}

void ASCOMDevice::command_blind(const std::string &command_name, bool raw)
{
    put("commandblind", {command_name, raw});
}

const bool ASCOMDevice::command_bool(const std::string &command_name, bool raw)
{
    return stringToBool(put("commandbool", {command_name, raw}));
}

const std::string ASCOMDevice::command_string(const std::string &command_name, bool raw)
{
    return put("commandstring", {command_name, raw});
}

const bool ASCOMDevice::getConnected()
{
    return stringToBool(get("connected"));
}

const void ASCOMDevice::setConnected(bool connecte_state)
{
    put("connected", {connecte_state});
}

const std::string ASCOMDevice::getDescription()
{
    return get("description");
}

const std::vector<std::string> ASCOMDevice::getDriverInfo()
{
    char delimiter = ',';
    return splitString(get("driverinfo"), delimiter);
}

const std::string ASCOMDevice::getDriverVersion()
{
    return get("driverversion");
}

const int ASCOMDevice::getInterfaceVersion()
{
    return std::stoi(get("interfaceversion"));
}

const std::string ASCOMDevice::getName()
{
    return get("name");
}

const std::vector<std::string> ASCOMDevice::getSupportedActions()
{
    return {};
}

const std::string ASCOMDevice::get(const std::string &attribute, const json &data, double tmo)
{
    // Make Host: header safe for IPv6
    httplib::Headers hdrs;
    if (address[0] == '[' && address.find("[::1]") != 0)
    {
        hdrs = {{"Host", address.substr(0, address.find('%')) + "]"}};
    }

    json pdata = {
        {"ClientTransactionID", client_trans_id},
        {"ClientID", client_id}};
    pdata.update(data);

    try
    {
        std::lock_guard<std::mutex> lock(ctid_lock);

        auto response = rqs.Get(std::format("{}/{}", base_url, attribute));
        client_trans_id++;

        check_error(response);
        return response->body;
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Exception occurred during GET request: %s", e.what());
        throw;
    }
}

const std::string ASCOMDevice::put(const std::string &attribute, const json &data, double tmo)
{
    // Make Host: header safe for IPv6
    httplib::Headers hdrs;
    if (address[0] == '[' && address.find("[::1]") != 0)
    {
        hdrs = {{"Host", address.substr(0, address.find('%')) + "]"}};
    }

    json pdata = {
        {"ClientTransactionID", client_trans_id},
        {"ClientID", client_id}};
    pdata.update(data);

    try
    {
        std::lock_guard<std::mutex> lock(ctid_lock);
        auto response = rqs.Put((base_url + '/' + attribute).c_str(), hdrs, pdata.dump(), "application/json");
        client_trans_id++;

        check_error(response);
        return response->body;
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Exception occurred during PUT request: %s", e.what());
        throw;
    }
}

void ASCOMDevice::check_error(const httplib::Result &response)
{
    int status_code = response->status;
    if (status_code >= 200 && status_code < 204)
    {
        json j = json::parse(response->body);
        int error_number = j["ErrorNumber"];
        std::string error_message = j["ErrorMessage"];

        if (error_number != 0)
        {
            switch (error_number)
            {
            case 0x0400:
                throw NotImplementedException(error_message);
            case 0x0401:
                throw InvalidValueException(error_message);
            case 0x0402:
                throw ValueNotSetException(error_message);
            case 0x0407:
                throw NotConnectedException(error_message);
            case 0x0408:
                throw ParkedException(error_message);
            case 0x0409:
                throw SlavedException(error_message);
            case 0x040B:
                throw InvalidOperationException(error_message);
            case 0x040c:
                throw ActionNotImplementedException(error_message);
            default:
                throw DriverException(error_number, error_message);
            }
        }
    }
    else
    {
        throw std::runtime_error("Alpaca request failed with status code " + std::to_string(status_code));
    }
}