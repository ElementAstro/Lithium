/*
 * daemon_task.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Daemon Task Definition

**************************************************/

#include "daemon_task.hpp"

DaemonTask::DaemonTask(const std::function<void()> &task_fn,
                       std::function<json(const json &)> &stop_fn)
    : BasicTask(stop_fn, true), task_fn_(task_fn)
{
}

const json DaemonTask::execute()
{
    // 执行任务的具体逻辑实现
#if __cplusplus >= 202002L
    std::jthread task_thread([this](std::stop_token stoken)
                             {
#else
    std::jthread task_thread([this](std::stop_token stoken)
                             {
#endif
        while (!stoken.stop_requested())
        {
            if (task_fn_)
            {
                task_fn_();
            }
        }
        done_ = true; });
    task_thread.detach(); // 分离任务线程
    return {{"status", "running"}};
}

const json DaemonTask::toJson() const
{
    auto json = BasicTask::toJson();
    json["type"] = "daemon";
    return json;
}

void DaemonTask::runTask()
{
    while (!stop_flag_)
    {
        task_fn_();
    }
    done_ = true;
}
