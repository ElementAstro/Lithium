/*
 * camera_task.hpp
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

Date: 2023-3-28

Description: Define All of the Camera Tasks

**************************************************/

#pragma once

#include "define.hpp"
#include "device/manager.hpp"

#include "nlohmann/json.hpp"

namespace OpenAPT
{
    class SingleShotTask : public SimpleTask 
    {
        public:
            SingleShotTask(const std::function<void(const nlohmann::json &)> &func, const nlohmann::json &params);
            ~SingleShotTask() {}

        private:
            std::function<void(const nlohmann::json &)> m_func;
            nlohmann::json m_params;
    };
} // namespace OpenAPT
