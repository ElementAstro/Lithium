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
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium {
using MacroValue =
    std::variant<std::string, json,
                 std::function<json(const std::vector<std::string>&)>>;
class TaskGenerator {
public:
    TaskGenerator();

    void add_macro(const std::string& name, const MacroValue& value);
    void process_json(json& j) const;
    void process_json_with_json_macros(json& j);

private:
    std::unordered_map<std::string, MacroValue> macros;

    std::string evaluate_macro(const std::string& name,
                               const std::vector<std::string>& args) const;
    std::string replace_macros(const std::string& input) const;
};

}  // namespace lithium
#endif
