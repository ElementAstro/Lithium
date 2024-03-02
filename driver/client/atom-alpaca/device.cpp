/*
 * device.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: Basic Device Definition of Alpaca

**************************************************/

#include "device.hpp"

#include <chrono>
#include <iostream>
#include <random>


#include "httplib.h"

constexpr int API_VERSION = 1;

Device::Device(const std::string &address, const std::string &device_type,
               int device_number, const std::string &protocol)
    : address(address),
      device_type(device_type),
      device_number(device_number),
      protocol(protocol) {
    api_version = API_VERSION;
    base_url = protocol + "://" + address + "/api/v" +
               std::to_string(api_version) + "/" + device_type + "/" +
               std::to_string(device_number);
}

std::string Device::Action(const std::string &ActionName,
                           const std::vector<std::string> &Parameters) {
    return _put("action", {{"Action", ActionName}, {"Parameters", Parameters}})
        .at("Value");
}

void Device::CommandBlind(const std::string &Command, bool Raw) {
    _put("commandblind", {{"Command", Command}, {"Raw", Raw}});
}

bool Device::CommandBool(const std::string &Command, bool Raw) {
    return _put("commandbool", {{"Command", Command}, {"Raw", Raw}})
        .at("Value");
}

std::string Device::CommandString(const std::string &Command, bool Raw) {
    return _put("commandstring", {{"Command", Command}, {"Raw", Raw}})
        .at("Value");
}

bool Device::get_Connected() const { return _get("connected") == "true"; }

void Device::set_Connected(bool ConnectedState) {
    _put("connected", {{"Connected", ConnectedState}});
}

std::string Device::get_Description() const { return _get("description"); }

std::vector<std::string> Device::get_DriverInfo() const {
    std::vector<std::string> driver_info;
    std::string driver_info_str = _get("driverinfo");
    size_t start_pos = 0;
    size_t end_pos = driver_info_str.find(',');
    while (end_pos != std::string::npos) {
        driver_info.push_back(
            driver_info_str.substr(start_pos, end_pos - start_pos));
        start_pos = end_pos + 1;
        end_pos = driver_info_str.find(',', start_pos);
    }
    driver_info.push_back(driver_info_str.substr(start_pos));
    return driver_info;
}

std::string Device::get_DriverVersion() const { return _get("driverversion"); }

int Device::get_InterfaceVersion() const {
    return std::stoi(_get("interfaceversion"));
}

std::string Device::get_Name() const { return _get("name"); }

std::vector<std::string> Device::get_SupportedActions() const {
    std::string supported_actions_str = _get("supportedactions");
    std::vector<std::string> supported_actions;
    size_t start_pos = 0;
    size_t end_pos = supported_actions_str.find(',');
    while (end_pos != std::string::npos) {
        supported_actions.push_back(
            supported_actions_str.substr(start_pos, end_pos - start_pos));
        start_pos = end_pos + 1;
        end_pos = supported_actions_str.find(',', start_pos);
    }
    supported_actions.push_back(supported_actions_str.substr(start_pos));
    return supported_actions;
}

json Device::_get(const std::string &attribute,
                  const std::map<std::string, std::string> &data = {},
                  double tmo = 5.0) const {
    httplib::Client client(address.c_str());
    std::string path = base_url + "/" + attribute;

    std::lock_guard<std::mutex> lock(_ctid_lock);
    std::string client_trans_id = std::to_string(_client_trans_id++);
    std::string client_id = std::to_string(_client_id);

    client.set_timeout(tmo);

    httplib::Headers headers = {{"ClientTransactionID", client_trans_id},
                                {"ClientID", client_id}};

    httplib::Result response = client.Get(path.c_str(), headers, data);

    if (response && response->status == 200) {
        json j = json::parse(response->body);
        int error_number = j["ErrorNumber"];
        std::string error_message = j["ErrorMessage"];
        if (error_number != 0) {
            if (error_number == 0x0400)
                throw NotImplementedException(error_message);
            else if (error_number == 0x0401)
                throw InvalidValueException(error_message);
            else if (error_number == 0x0402)
                throw ValueNotSetException(error_message);
            else if (error_number == 0x0407)
                throw NotConnectedException(error_message);
            else if (error_number == 0x0408)
                throw ParkedException(error_message);
            else if (error_number == 0x0409)
                throw SlavedException(error_message);
            else if (error_number == 0x040B)
                throw InvalidOperationException(error_message);
            else if (error_number == 0x040C)
                throw ActionNotImplementedException(error_message);
            else if (error_number >= 0x500 && error_number <= 0xFFF)
                throw DriverException(error_number, error_message);
            else
                throw DriverException(error_number, error_message);
        }
        return j;
    } else {
        throw AlpacaRequestException(
            response ? response->status : -1,
            response ? response->body : "Request failed");
    }
}

json Device::_put(const std::string &attribute,
                  const std::map<std::string, std::any> &data = {},
                  double tmo = 5.0) const {
    httplib::Client client(address.c_str());
    std::string path = base_url + "/" + attribute;

    std::lock_guard<std::mutex> lock(_ctid_lock);
    std::string client_trans_id = std::to_string(_client_trans_id++);
    std::string client_id = std::to_string(_client_id);

    client.set_timeout(tmo);

    httplib::Headers headers = {{"ClientTransactionID", client_trans_id},
                                {"ClientID", client_id}};

    json json_data = data;
    std::string body = json_data.dump();

    httplib::Result response =
        client.Put(path.c_str(), headers, body, "application/json");

    if (response && response->status == 200) {
        json j = json::parse(response->body);
        int error_number = j["ErrorNumber"];
        std::string error_message = j["ErrorMessage"];
        if (error_number != 0) {
            if (error_number == 0x0400)
                throw NotImplementedException(error_message);
            else if (error_number == 0x0401)
                throw InvalidValueException(error_message);
            else if (error_number == 0x0402)
                throw ValueNotSetException(error_message);
            else if (error_number == 0x0407)
                throw NotConnectedException(error_message);
            else if (error_number == 0x0408)
                throw ParkedException(error_message);
            else if (error_number == 0x0409)
                throw SlavedException(error_message);
            else if (error_number == 0x040B)
                throw InvalidOperationException(error_message);
            else if (error_number == 0x040C)
                throw ActionNotImplementedException(error_message);
            else if (error_number >= 0x500 && error_number <= 0xFFF)
                throw DriverException(error_number, error_message);
            else
                throw DriverException(error_number, error_message);
        }
        return j;
    } else {
        throw AlpacaRequestException(
            response ? response->status : -1,
            response ? response->body : "Request failed");
    }
}

int Device::_client_id = std::random_device()();
int Device::_client_trans_id = 1;
std::mutex Device::_ctid_lock;
