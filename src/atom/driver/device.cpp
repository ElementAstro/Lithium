/*
 * device.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Defination

*************************************************/

#include "device.hpp"

#include "atom/experiment/decorate.hpp"
#include "atom/utils/uuid.hpp"

#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif
#include <typeindex>
#include <typeinfo>

AtomDriver::AtomDriver(const std::string &name)
    : SharedComponent(), m_name(name) {
    Atom::Utils::UUIDGenerator generator;
    m_uuid = generator.generateUUID();
    SetVariable("uuid", m_uuid);
    SetVariable("name", m_name);
}

AtomDriver::~AtomDriver() {}

bool AtomDriver::connect(const json &params) { return true; };

bool AtomDriver::disconnect(const json &params) { return true; };

bool AtomDriver::reconnect(const json &params) { return true; };

bool AtomDriver::isConnected() { return true; }

json AtomDriver::Connect(const json &params) {
    if (!params.contains("name")) {
        return {{"command" : }};
    }
    auto d_connect = make_decorator()
}

json AtomDriver::Disconnect(const json &params) {}

json AtomDriver::Reconnect(const json &params) {}

json AtomDriver::IsConnected(const json &params) {}
