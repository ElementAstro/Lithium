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

LoopTask::LoopTask(const std::function<void(const json &)> &item_fn,
                   const json &params,
                   std::function<json(const json &)> &stop_fn)
    : BasicTask(stop_fn, stop_fn != nullptr), item_fn_(item_fn), params_(params) {}

const json LoopTask::execute()
{
    for (const auto &item : params_["items"])
    {
        if (stop_flag_)
        {
            break;
        }
        item_fn_({{"item", item}});
    }
    done_ = true;
    return {{"status", "done"}};
}

const json LoopTask::toJson() const
{
    auto json = BasicTask::toJson();
    json["type"] = "loop";
    json["params"] = params_;
    return json;
}