/*
 * conditional_task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Conditional Task Definition

**************************************************/

#pragma once

#include "task.hpp"

class ConditionalTask : public Atom::Task::BasicTask
{
public:
    /**
     * @brief ConditionalTask类构造函数
     * @param condition_fn 条件函数，用于判断是否执行任务
     * @param params 任务参数
     * @param task_fn 任务函数，用于执行任务逻辑
     * @param stop_fn 一个可选的停止函数，默认为nullptr
     */
    ConditionalTask(const std::function<bool(const json &)> &condition_fn,
                    const json &params,
                    const std::function<void(const json &)> &task_fn,
                    std::function<json(const json &)> &stop_fn);

    /**
     * @brief 执行任务的虚函数，由子类实现具体逻辑
     * @return 以json格式返回任务执行结果
     */
    virtual const json execute() override;

    /**
     * @brief 将任务序列化为JSON对象
     * @return 表示任务的JSON对象
     */
    virtual const json toJson() const override;

private:
    // 条件函数，用于判断是否执行任务
    std::function<bool(const json &)> condition_fn_;

    // 任务参数
    json params_;

    // 任务函数，用于执行任务逻辑
    std::function<void(const json &)> task_fn_;
};