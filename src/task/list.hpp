/*
 * list.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Thread safe task list

**************************************************/

#ifndef LITHIUM_TASK_LIST_HPP
#define LITHIUM_TASK_LIST_HPP

#include <optional>
#include <shared_mutex>
#include <string>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {
class TaskList {
public:
    static std::shared_ptr<TaskList> createShared();

    bool addOrUpdateTask(const std::string &name, const json &params);

    bool insertTask(const std::string &name, const json &params,
                    const int &position);

    bool removeTask(const std::string &name);

    std::optional<json> getTaskParams(const std::string &name) const;

    void listTasks() const;

    std::unordered_map<std::string, json> getTasks();

private:
    mutable std::shared_mutex mtx;
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, json> tasks;
#else
    std::unordered_map<std::string, json> tasks;
#endif
};
}  // namespace lithium

#endif