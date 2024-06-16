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
#include <fstream>
#include <optional>
#include <string>
#include <vector>
#include <variant>

#include "atom/task/task.hpp"

using json = nlohmann::json;

namespace lithium {
class TaskGenerator {
public:
    using MacroValue =
        std::variant<std::string, std::function<std::string(
                                      const std::vector<std::string>&)>>;

    TaskGenerator();

    void add_macro(const std::string& name, const MacroValue& value);
    void process_json(json& j) const;

private:
    std::unordered_map<std::string, MacroValue> macros;

    std::string evaluate_macro(const std::string& name,
                               const std::vector<std::string>& args) const;
    std::string replace_macros(const std::string& input) const;
};

}  // namespace lithium
#endif
