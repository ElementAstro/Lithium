/*
 * loop_task.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

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