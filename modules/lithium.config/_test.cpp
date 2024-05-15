/*
 * _test.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Test Script

**************************************************/

#include "_component.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    auto config = std::make_shared<ConfigComponent>("lithium.config");
    json test_value = {{"key", "value"}};
    auto result =
        config->dispatch("getConfig", std::string("config/server/host"));
    try {
        std::cout << std::any_cast<std::optional<json>>(result).value().dump()
                  << std::endl;
    } catch (const std::bad_any_cast& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    std::cout << "Hello, World!" << std::endl;
    return 0;
}