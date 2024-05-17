/*
 * task_manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Manager

**************************************************/

#include "manager.hpp"

#include "addon/manager.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "utils/constant.hpp"

namespace lithium {
TaskManager::TaskManager() : m_StopFlag(false) {
    // Load Task Component from Global Ptr Manager
    m_TaskContainer =
        GetWeakPtr<TaskContainer>(constants::LITHIUM_TASK_CONTAINER);
    m_TaskPool = GetWeakPtr<TaskPool>(constants::LITHIUM_TASK_POOL);
    m_TaskList = GetWeakPtr<TaskList>(constants::LITHIUM_TASK_LIST);
    m_TaskGenerator =
        GetWeakPtr<TaskGenerator>(constants::LITHIUM_TASK_GENERATOR);
    m_TickScheduler = GetWeakPtr<TickScheduler>("lithium.task.tick");
    m_TaskLoader = GetWeakPtr<TaskLoader>("ltihium.task.loader");

    m_ComponentManager =
        GetWeakPtr<ComponentManager>(constants::LITHIUM_COMPONENT_MANAGER);
}

TaskManager::~TaskManager() { saveTasksToJson(); }

std::shared_ptr<TaskManager> TaskManager::createShared() {
    return std::make_shared<TaskManager>();
}

bool TaskManager::addTask(const std::string name, const json& params) {
    // Task Check Needed
    m_TaskList.lock()->addOrUpdateTask(name, params);
    return true;
}

bool TaskManager::insertTask(const int& position, const std::string& name,
                             const json& params) {
    if (m_TaskList.lock()->insertTask(name, params, position)) {
        DLOG_F(INFO, "Insert {} task to {}", name, position);
    } else {
    }
    return true;
}

bool TaskManager::modifyTask(const std::string& name, const json& params) {
    m_TaskList.lock()->addOrUpdateTask(name, params);
    return true;
}

bool TaskManager::deleteTask(const std::string& name) {
    m_TaskList.lock()->removeTask(name);
    return true;
}

bool TaskManager::executeAllTasks() {
    for (const auto& [name, params] : m_TaskList.lock()->getTasks()) {
        DLOG_F(INFO, "Run task {}", name);
        std::string task_type = params["type"].get<std::string>();
        if (auto task = m_TaskContainer.lock()->getTask(task_type);
            task.has_value()) {
            json t_params = params["params"];
            auto handle = m_TickScheduler.lock()->scheduleTask(
                1, false, 1, std::chrono::milliseconds(0), std::nullopt,
                std::nullopt, std::nullopt, task.value()->m_function, t_params);

            if (params.contains("callbacks")) {
                std::vector<std::string> callbacks = params["callbacks"];
                for (auto callback : callbacks) {
                    auto c_task = m_TaskContainer.lock()->getTask(task_type);
                    if (c_task.has_value()) {
                        m_TickScheduler.lock()->setCompletionCallback(
                            handle,
                            [c_task]() { c_task.value()->m_function({}); });
                    }
                }
            }
            if (params.contains("timers")) {
                std::vector<json> timers = params["timers"];
                for (auto timer : timers) {
                    if (!timer.contains("name") || !timer.contains("params") ||
                        !timer.contains("delay")) {
                        continue;
                    } else {
                        std::string timer_name = timer["name"];
                        int tick = timer["delay"];
                        if (auto tt_task =
                                m_TaskContainer.lock()->getTask(name);
                            tt_task.has_value()) {
                            m_TickScheduler.lock()->scheduleTask(
                                tick, false, 1, std::chrono::milliseconds(0),
                                std::nullopt, std::nullopt, std::nullopt,
                                tt_task.value()->m_function, timer["params"]);
                        }
                    }
                }
            }
            m_Timer->start();
            while (!handle->completed.load() || !m_StopFlag.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (params.contains("timeout") &&
                    m_Timer->elapsedSeconds() >
                        params["timeout"].get<double>()) {
                    LOG_F(ERROR, "Timeout");
                    break;
                }
            }
            m_Timer->stop();
            m_Timer->reset();
        } else {
            LOG_F(ERROR, "Task {} contains a invalid function target");
        }
        if (m_StopFlag.load()) {
            break;
        }
    }
    return true;
}

void TaskManager::stopTask() { m_StopFlag.store(true); }

bool TaskManager::executeTaskByName(const std::string& name) { return false; }

bool TaskManager::saveTasksToJson() const { return true; }

// 设置全局变量
void TaskManager::set_global_var(const std::string& name,
                                 const BoxedValue& value) {
    m_global_vars[name] = value;
}

// 获取全局变量
BoxedValue TaskManager::get_global_var(const std::string& name) {
    return m_global_vars[name];
}

// 设置局部变量
void TaskManager::set_local_var(const std::string& name,
                                const BoxedValue& value) {
    m_local_vars[name] = value;
}

// 获取局部变量
BoxedValue TaskManager::get_local_var(const std::string& name) {
    if (m_local_vars.find(name) == m_local_vars.end())
        return BoxedValue();
    return m_local_vars[name];
}

void TaskManager::execute_task(const json& task) {
    size_t i = 0;
    while (i < task.size()) {
        const auto& statement = task[i];
        const std::string& type = statement.at("type");

        if (type == "function") {
            std::string name = statement.at("name");
            json params = statement.at("params");
            if (!params.is_null()) {
                for (auto& param : params) {
                    if (param.is_object() && param.contains("name")) {
                        auto param_name = param.at("name").get<std::string>();
                        std::cout << param_name << std::endl;
                        if (auto var = get_local_var(param_name);
                            !var.is_null()) {
                            auto var_s = var.get();
                            if (var_s.type() == typeid(std::string)) {
                                param = std::any_cast<std::string>(var);
                            } else if (var_s.type() == typeid(int)) {
                                param = std::any_cast<int>(var);
                            } else if (var_s.type() == typeid(double)) {
                                param = std::any_cast<double>(var);
                            } else if (var_s.type() == typeid(bool)) {
                                param = std::any_cast<bool>(var);
                            } else if (var_s.type() ==
                                       typeid(std::vector<int>)) {
                                param = std::any_cast<std::vector<int>>(var);
                            } else if (var_s.type() ==
                                       typeid(std::vector<double>)) {
                                param = std::any_cast<std::vector<double>>(var);
                            } else if (var_s.type() ==
                                       typeid(std::vector<std::string>)) {
                                param = std::any_cast<std::vector<std::string>>(
                                    var);
                            } else {
                                std::cout << "Variable type not supported"
                                          << std::endl;
                            }
                        } else if (var = get_global_var(param_name);
                                   !var.is_null()) {
                        } else {
                            std::cout << "Variable not found" << std::endl;
                        }
                    }
                }
            }
            if (m_CommandDispatcher.lock()->has(name)) {
                m_CommandDispatcher.lock()->dispatch(name, params);
            }
        } else if (type == "if") {
            execute_if(statement);
        } else if (type == "while") {
            execute_while(statement);
        } else if (type == "for") {
            execute_for(statement);
        } else if (type == "goto") {
            execute_goto(statement, const_cast<json&>(task), i);
            continue;  // 防止 i 被自增
        } else if (type == "set_var") {
            std::string var_name = statement.at("name");
            std::any var_value = execute_expression(statement.at("value"));
            set_local_var(var_name, BoxedValue(var_value));
        }
        ++i;
    }
}

// In the execute_expression method, ensure that variable values are
// correctly fetched
std::any TaskManager::execute_expression(const json& expression) {
    std::string type = expression.at("type");
    if (type == "literal") {
        auto& value = expression.at("value");
        if (value.is_number_float()) {
            return value.get<double>();
        } else if (value.is_number_integer()) {
            return value.get<int>();
        } else if (value.is_string()) {
            return value.get<std::string>();
        } else {
            return nullptr;
        }
    } else if (type == "variable") {
        std::string var_name = expression.at("name");
        return resolve_variable(var_name);
    } else if (type == "function") {
        std::string func_name = expression.at("name");
        json params = expression.at("params");
        if (m_CommandDispatcher.lock()->has(func_name)) {
            json resolved_params = json::object();
            for (auto& [key, val] : params.items()) {
                std::cout << key << " " << val.dump() << std::endl;
                resolved_params[key] =
                    std::any_cast<json>(execute_expression(val));
            }
            return m_CommandDispatcher.lock()->dispatch(func_name,
                                                        resolved_params);
        }
    } else if (type == "binary_op") {
        std::string op = expression.at("op");
        std::any left = execute_expression(expression.at("left"));
        std::any right = execute_expression(expression.at("right"));

        // Ensure we handle both int and double types properly
        if (left.type() == typeid(int) && right.type() == typeid(int)) {
            int lval = std::any_cast<int>(left);
            int rval = std::any_cast<int>(right);
            return execute_binary_op(lval, rval, op);
        } else if ((left.type() == typeid(double) ||
                    left.type() == typeid(int)) &&
                   (right.type() == typeid(double) ||
                    right.type() == typeid(int))) {
            double lval = std::any_cast<double>(left);
            double rval = std::any_cast<double>(right);
            return execute_binary_op(lval, rval, op);
        }
    }
    return nullptr;
}

// Utility to resolve variable from local or global scope
std::any TaskManager::resolve_variable(const std::string& var_name) {
    if (m_local_vars.find(var_name) != m_local_vars.end()) {
        return m_local_vars[var_name];
    } else if (m_global_vars.find(var_name) != m_global_vars.end()) {
        return m_global_vars[var_name];
    }
    std::cerr << "Variable " << var_name << " not found." << std::endl;
    return nullptr;  // Variable not found
}

// Enhanced execute_if to support complex conditions
void TaskManager::execute_if(const json& statement) {
    if (evaluate_condition(statement.at("condition"))) {
        execute_task(statement.at("then"));
    } else if (statement.contains("else")) {
        execute_task(statement.at("else"));
    }
}

// Function to evaluate conditions based on JSON description
bool TaskManager::evaluate_condition(const json& condition) {
    std::string op = condition.at("op").get<std::string>();

    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" ||
        op == ">=") {
        std::any left = execute_expression(condition.at("left"));
        std::any right = execute_expression(condition.at("right"));

        // Perform comparison based on the operand type (int, double,
        // string)
        if (left.type() == typeid(int) && right.type() == typeid(int)) {
            int lval = std::any_cast<int>(left);
            int rval = std::any_cast<int>(right);
            return compare_values(lval, rval, op);
        } else if (left.type() == typeid(double) &&
                   right.type() == typeid(double)) {
            double lval = std::any_cast<double>(left);
            double rval = std::any_cast<double>(right);
            return compare_values(lval, rval, op);
        } else if (left.type() == typeid(std::string) &&
                   right.type() == typeid(std::string)) {
            std::string lval = std::any_cast<std::string>(left);
            std::string rval = std::any_cast<std::string>(right);
            if (op == "==")
                return lval == rval;
            else if (op == "!=")
                return lval != rval;
            else
                std::cerr << "Unsupported operation for strings: " << op
                          << std::endl;
        }
    } else if (op == "range") {
        // Range check: {"op": "range", "value": ..., "low": ...,
        // "high":
        // ...}
        std::any value = execute_expression(condition.at("value"));
        std::any low = execute_expression(condition.at("low"));
        std::any high = execute_expression(condition.at("high"));
        if (value.type() == typeid(int)) {
            int v = std::any_cast<int>(value);
            int l = std::any_cast<int>(low);
            int h = std::any_cast<int>(high);
            return l <= v && v <= h;
        } else if (value.type() == typeid(double)) {
            double v = std::any_cast<double>(value);
            double l = std::any_cast<double>(low);
            double h = std::any_cast<double>(high);
            return l <= v && v <= h;
        }
    } else if (op == "contains") {
        // String contains: {"op": "contains", "value": ...,
        // "substring":
        // ...}
        std::any value = execute_expression(condition.at("value"));
        std::any substring = execute_expression(condition.at("substring"));
        if (value.type() == typeid(std::string) &&
            substring.type() == typeid(std::string)) {
            std::string v = std::any_cast<std::string>(value);
            std::string s = std::any_cast<std::string>(substring);
            return v.find(s) != std::string::npos;
        }
    }
    return false;
}

// 解析循环语句
void TaskManager::execute_while(const json& statement) {
    while (evaluate_condition(statement.at("condition"))) {
        execute_task(statement.at("body"));
    }
}

// 解析 for 语句
void TaskManager::execute_for(const json& statement) {
    std::any start = execute_expression(statement.at("start"));
    std::any end = execute_expression(statement.at("end"));
    std::any step = statement.contains("step")
                        ? execute_expression(statement.at("step"))
                        : 1;
    int start_val = std::any_cast<int>(start);
    int end_val = std::any_cast<int>(end);
    int step_val = std::any_cast<int>(step);
    for (int i = start_val; i < end_val; i += step_val) {
        execute_task(statement.at("body"));
    }
}

// 解析 goto 语句
void TaskManager::execute_goto(const json& statement, json& task, size_t& i) {
    std::string label = statement.at("label");
    for (i = 0; i < task.size(); ++i) {
        if (task[i].contains("label") && task[i].at("label") == label) {
            break;
        }
    }
}
}  // namespace lithium
