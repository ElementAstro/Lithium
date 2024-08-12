/*
 * _main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-26

Description: Main Entry

**************************************************/

#include "_component.hpp"

#include "atom/type/json.hpp"
#include "macro.hpp"
using json = nlohmann::json;

ATOM_C {
auto getInstance(ATOM_UNUSED const json &params) -> std::shared_ptr<Component> {
    if (params.contains("name") && params["name"].is_string()) {
        return std::make_shared<ToolsComponent>(
            params["name"].get<std::string>());
    }
    return std::make_shared<ToolsComponent>("lithium.cxxtools");
}
}
