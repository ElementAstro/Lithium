/*
 * _main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-16

Description: Main Entry

**************************************************/

#include "_component.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

extern "C" {
std::shared_ptr<Component> getInstance([[maybe_unused]] const json &params) {
    if (params.contains("name") && params["name"].is_string()) {
        return std::make_shared<INDIServerComponent>(
            params["name"].get<std::string>());
    }
    return std::make_shared<INDIServerComponent>("lithium.indiserver");
}
}
