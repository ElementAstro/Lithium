/*
 * _main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-07-20

Description: Main Entry

**************************************************/

#include "_component.hpp"
#include "astrometry.hpp"

extern "C" {
auto getInstance(std::string name) -> std::shared_ptr<Component> {
    return std::make_shared<AstrometryComponent>(
        name.empty() ? "lithium.client.astrometry" : name);
}
}
