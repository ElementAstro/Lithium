/*
 * loop_task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Loop Task Definition

**************************************************/

#pragma once

#include "task.hpp"

class LoopTask : public Atom::Task::BasicTask
{
public:
    /**
     * @brief LoopTask类构造函数
     * @param item_fn 单项任务函数，对每个参数执行一次
     * @param params 任务参数
     * @param stop_fn 一个可选的停止函数，默认为nullptr
     */
    LoopTask(const std::function<void(const json &)> &item_fn,
             const json &params,
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
    // 单项任务函数，对每个参数执行一次
    std::function<void(const json &)> item_fn_;

    // 任务参数
    json params_;
};