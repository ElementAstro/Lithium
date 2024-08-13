/**
 * @file generator.hpp
 * @brief Task Generator
 *
 * This file contains the definition and implementation of a task generator.
 *
 * @date 2023-07-21
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef LITHIUM_TASK_GENERATOR_HPP
#define LITHIUM_TASK_GENERATOR_HPP

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium {
using MacroValue =
    std::variant<std::string,
                 std::function<std::string(const std::vector<std::string>&)>>;

class TaskGenerator {
public:
    TaskGenerator();
    ~TaskGenerator();

    static auto createShared() -> std::shared_ptr<TaskGenerator>;

    void addMacro(const std::string& name, MacroValue value);
    void processJson(json& j) const;
    void processJsonWithJsonMacros(json& j);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;  // Pimpl for encapsulation
};

}  // namespace lithium

#endif  // LITHIUM_TASK_GENERATOR_HPP
