/*
 * focuser.hpp
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

Description: Basic Focuser Defination

*************************************************/

#pragma once

#include "device.hpp"

class Focuser : public Device
{
public:
    /**
     * @brief 构造函数，创建一个名为 name 的电调对象
     *
     * @param name 电调名称
     */
    Focuser(const std::string &name);

    /**
     * @brief 析构函数，释放资源
     */
    virtual ~Focuser();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

    /**
     * @brief 将电调移动到 position 位置
     *
     * @param position 相对移动的步数
     * @return bool 移动是否成功
     */
    virtual bool moveTo(const json &params);

    /**
     * @brief 将电调移动到绝对位置 position
     *
     * @param position 绝对位置步数
     * @return bool 移动是否成功
     */
    virtual bool moveToAbsolute(const json &params);

    /**
     * @brief 移动电调 step 个步长
     *
     * @param step 移动步数
     * @return bool 移动是否成功
     */
    virtual bool moveStep(const json &params);

    /**
     * @brief 移动电调至绝对步数位置
     *
     * @param step 绝对步数位置
     * @return bool 移动是否成功
     */
    virtual bool moveStepAbsolute(const json &params);

    /**
     * @brief 中止电调移动
     *
     * @return bool 操作是否成功
     */
    virtual bool AbortMove(const json &params);

    /**
     * @brief 获取电调最大位置
     *
     * @return int 电调最大位置
     */
    virtual int getMaxPosition(const json &params);

    /**
     * @brief 设置电调最大位置
     *
     * @param max_position 电调最大位置
     * @return bool 操作是否成功
     */
    virtual bool setMaxPosition(const json &params);

    /**
     * @brief 判断是否支持获取温度功能
     *
     * @return bool 是否支持获取温度功能
     */
    virtual bool isGetTemperatureAvailable(const json &params);

    /**
     * @brief 获取电调当前温度
     *
     * @return double 当前温度
     */
    virtual double getTemperature(const json &params);

    /**
     * @brief 判断是否支持绝对移动功能
     *
     * @return bool 是否支持绝对移动功能
     */
    virtual bool isAbsoluteMoveAvailable(const json &params);

    /**
     * @brief 判断是否支持手动移动功能
     *
     * @return bool 是否支持手动移动功能
     */
    virtual bool isManualMoveAvailable(const json &params);

    /**
     * @brief 获取电调当前位置
     *
     * @return int 当前位置
     */
    virtual int getCurrentPosition(const json &params);

    /**
     * @brief 判断电调是否存在反向间隙
     *
     * @return bool 是否存在反向间隙
     */
    virtual bool haveBacklash(const json &params);

    /**
     * @brief 设置电调反向间隙值
     *
     * @param value 反向间隙值
     * @return bool 操作是否成功
     */
    virtual bool setBacklash(const json &params);
};