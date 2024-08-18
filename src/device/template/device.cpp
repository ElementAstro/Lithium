/*
 * device.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Defintion

*************************************************/

#include "device.hpp"

#include "atom/utils/uuid.hpp"

AtomDriver::AtomDriver(std::string name)
    : name_(name), uuid_(atom::utils::UUID().toString()) {}

auto AtomDriver::getUUID() const -> std::string { return uuid_; }

auto AtomDriver::getName() const -> std::string { return name_; }

void AtomDriver::setName(const std::string& newName) { name_ = newName; }

auto AtomDriver::getType() const -> std::string { return type_; }
