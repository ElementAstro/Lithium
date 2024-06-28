/*
 * generator.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Generator

**************************************************/

#ifndef LITHIUM_TASK_GENERATOR_HPP
#define LITHIUM_TASK_GENERATOR_HPP

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <functional>
#include <string>
#include <variant>
#include <vector>

#include "atom/type/json_fwd.hpp"

namespace lithium {
using MacroValue = std::variant<
    std::string, nlohmann::json,
    std::function<nlohmann::json(const std::vector<std::string>&)>>;
class TaskGenerator {
public:
    TaskGenerator();

    void addMacro(const std::string& name, const MacroValue& value);
    void processJson(nlohmann::json& j) const;
    void processJsonWithJsonMacros(nlohmann::json& j);

private:
    std::unordered_map<std::string, MacroValue> macros_;

    auto evaluateMacro(const std::string& name,
                               const std::vector<std::string>& args) const -> std::string;
    auto replaceMacros(const std::string& input) const -> std::string;
};

}  // namespace lithium
#endif
