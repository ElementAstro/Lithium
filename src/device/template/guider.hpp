/*
 * guider.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: AtomGuider Simulator and Basic Definition

*************************************************/

#pragma once

#include "device.hpp"

#include <optional>

class AtomGuider : public AtomDriver {
public:
    explicit AtomGuider(std::string name) : AtomDriver(name) {}

    
};
