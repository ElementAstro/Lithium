/*
 * daemon_task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Daemon Task Definition

**************************************************/

#pragma once

#include "task.hpp"

class DaemonTask : public Atom::Task::BasicTask
{
public:
    /**
     * @brief 构造函数
     * @param task_fn 任务函数
     * @param stop_fn 任务停止函数
     */
    DaemonTask(const std::function<void()> &task_fn,
               std::function<json(const json &)> &stop_fn);

    /**
     * @brief 执行任务
     * @return JSON对象表示的任务状态
     */
    const json execute() override;

    /**
     * @brief 将任务序列化为JSON对象
     * @return JSON对象表示的任务
     */
    const json toJson() const override;

private:
    std::function<void()> task_fn_; // 任务函数

    /**
     * @brief 在循环中运行任务的线程
     */
    void runTask();
};