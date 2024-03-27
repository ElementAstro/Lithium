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
    : SharedComponent(name), m_name(name) {}

AtomDriver::~AtomDriver() {}

bool AtomDriver::initialize() {
    SharedComponent::initialize();
    Atom::Utils::UUIDGenerator generator;
    m_uuid = generator.generateUUID();
    setVariable("DEVICE_UUID", m_uuid);
    setVariable("DEVICE_NAME", m_name);

    registerFunc("connect", &AtomDriver::Connect, this);
    registerFunc("disconnect", &AtomDriver::Disconnect, this);
    registerFunc("reconnect", &AtomDriver::Reconnect, this);
    registerFunc("isConnected", &AtomDriver::IsConnected, this);
    return true;
}

bool AtomDriver::connect(const json &params) { return true; };

bool AtomDriver::disconnect(const json &params) { return true; };

bool AtomDriver::reconnect(const json &params) { return true; };

bool AtomDriver::isConnected() { return true; }

json AtomDriver::Connect(const json &params) {
    if (!params.contains("name")) {
        return {{"command", ""}};
    }
    return {};
}

json AtomDriver::Disconnect(const json &params) { return {}; }

json AtomDriver::Reconnect(const json &params) { return {}; }

json AtomDriver::IsConnected(const json &params) { return {}; }
