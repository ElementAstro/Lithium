/*
 * _main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-18

Description: Main Entry

**************************************************/

#include "_component.hpp"
#include "macro.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

ATOM_EXTERN_C {
    auto getInstance(
        [[maybe_unused]] const json &params) -> std::shared_ptr<Component> {
        if (params.contains("name") && params["name"].is_string()) {
            return std::make_shared<SysInfoComponent>(
                params["name"].get<std::string>());
        }
        return std::make_shared<SysInfoComponent>("atom.sysinfo");
    }
}
