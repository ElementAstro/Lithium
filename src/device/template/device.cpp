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

#include "atom/utils/uuid.hpp"
#include "macro.hpp"

AtomDriver::AtomDriver(std::string name)
    : name_(name), uuid_(atom::utils::UUID().toString()) {}

AtomDriver::~AtomDriver() {}

auto AtomDriver::initialize() -> bool { return true; }

auto AtomDriver::destroy() -> bool { return true; }

std::string AtomDriver::getUUID() const { return uuid_; }

std::string AtomDriver::getName() const { return name_; }

void AtomDriver::setName(const std::string& newName) { name_ = newName; }

std::string AtomDriver::getType() const { return type_; }

auto AtomDriver::connect(const std::string& name, int timeout,
                         int maxRetry) -> bool {
    ATOM_UNREF_PARAM(name);
    ATOM_UNREF_PARAM(timeout);
    ATOM_UNREF_PARAM(maxRetry);
    return true;
}

auto AtomDriver::disconnect(bool force, int timeout, int maxRetry) -> bool {
    ATOM_UNREF_PARAM(force);
    ATOM_UNREF_PARAM(timeout);
    ATOM_UNREF_PARAM(maxRetry);
    return true;
}

auto AtomDriver::reconnect(int timeout, int maxRetry) -> bool {
    ATOM_UNREF_PARAM(timeout);
    ATOM_UNREF_PARAM(maxRetry);
    return true;
}

auto AtomDriver::scan() -> std::vector<std::string> { return {}; }

auto AtomDriver::isConnected() -> bool { return true; }
