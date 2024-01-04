/*
 * guider.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-6-1

Description: Basic Guider Defination

*************************************************/

#pragma once

#include "device.hpp"

class Guider : public Device
{
public:
    /**
     * @brief 构造函数，创建一个名为 name 的电调对象
     *
     * @param name 电调名称
     */
    Guider(const std::string &name);

    /**
     * @brief 析构函数，释放资源
     */
    virtual ~Guider();

    virtual bool connect(const nlohmann::json &params) override;

    virtual bool disconnect(const nlohmann::json &params) override;

    virtual bool reconnect(const nlohmann::json &params) override;
};