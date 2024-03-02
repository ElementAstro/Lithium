/*
 * loop_task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Loop Task Definition

**************************************************/

#include "loop_task.hpp"

LoopTask::LoopTask(const std::function<json(const json &)> &func,
                   const std::function<json(const json &)> &stop_fn,
                   const json &params_template, int loop_count)
    : SimpleTask(func, stop_fn, params_template), m_loopCount(loop_count) {}

json LoopTask::execute() {
    if (m_isExecuting.load()) {
        return {
            {"status", "error"}, {"error", "Task is executing"}, {"code", 400}};
    }
    if (m_loopCount == 0) {
        return {
            {"status", "error"}, {"error", "Loop count is 0"}, {"code", 400}};
    }
    if (m_loopCount < 0) {
        return {{"status", "error"},
                {"error", "Loop count is negative"},
                {"code", 400}};
    }

    int currentCount = 0;
    try {
        for (int i = 0; i < m_loopCount; i++) {
            m_isExecuting.store(true);
            currentCount++;
            if (m_stopFlag) {
                return {{"status", "error"},
                        {"error", "Task is stopped"},
                        {"code", 400},
                        {"count", currentCount}};
            }
            if (!m_paramsTemplate.is_null() && !m_params.is_null()) {
                if (!validateJsonValue(m_params, m_paramsTemplate)) {
                    return {{"status", "error"},
                            {"error", "Incorrect value type for element:"},
                            {"code", 500},
                            {"count", currentCount}};
                }
            }
            if (!m_stopFlag) {
                m_returns.push_back(m_function(m_params));
            }
            m_isExecuting.store(false);
        }
    } catch (const std::exception &e) {
        return {{"status", "error"},
                {"error", e.what()},
                {"code", 500},
                {"count", currentCount}};
    }

    return m_returns;
}

json LoopTask::toJson() {
    auto json = SimpleTask::toJson();
    json["type"] = "loop";
    json["loop"] = m_loopCount;
    return json;
}