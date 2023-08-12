/*
 * telescope.hpp
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

Date: 2023-6-1

Description: Basic Telescope Defination

*************************************************/

#pragma once

#include "device.hpp"

class Telescope : virtual public Device
{
public:
    /**
     * @brief 构造函数
     * @param name 望远镜名称
     */
    Telescope(const std::string &name);

    /**
     * @brief 析构函数
     */
    virtual ~Telescope();

    virtual bool connect(const std::string& name) override;

    virtual bool disconnect() override;

    virtual bool reconnect() override;

protected:
    /**
     * @brief 指向新目标
     * @param ra 目标赤经
     * @param dec 目标赤纬
     * @param j2000 是否使用J2000坐标系，默认为false，表示使用本地坐标系
     * @return 是否成功指向新目标
     */
    virtual bool SlewTo(const nlohmann::json &params);

    /**
     * @brief 中止望远镜的指向
     * @return 是否成功中止指向
     */
    virtual bool Abort(const nlohmann::json &params);

    /**
     * @brief 获取望远镜是否在指向新目标
     * @return 返回 true 表示正在指向新目标，否则返回 false
     */
    virtual bool isSlewing(const nlohmann::json &params);

    /**
     * @brief 获取当前赤经位置
     * @return 当前赤经位置
     */
    virtual std::string getCurrentRA(const nlohmann::json &params);

    /**
     * @brief 获取当前赤纬位置
     * @return 当前赤纬位置
     */
    virtual std::string getCurrentDec(const nlohmann::json &params);

    /**
     * @brief 开始跟踪运动目标
     * @param model 跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
     * @param speed 跟踪速度，默认为1
     * @return 是否成功开始跟踪运动目标
     */
    virtual bool StartTracking(const nlohmann::json &params);

    /**
     * @brief 停止跟踪运动目标
     * @return 是否成功停止跟踪运动目标
     */
    virtual bool StopTracking(const nlohmann::json &params);

    /**
     * @brief 设置跟踪模式
     * @param mode 跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
     * @return 是否成功设置跟踪模式
     */
    virtual bool setTrackingMode(const nlohmann::json &params);

    /**
     * @brief 设置跟踪速度
     * @param speed 跟踪速度
     * @return 是否成功设置跟踪速度
     */
    virtual bool setTrackingSpeed(const nlohmann::json &params);

    /**
     * @brief 获取当前跟踪模式
     * @return 当前跟踪模式，包括恒星跟踪、太阳跟踪和月球跟踪
     */
    virtual std::string getTrackingMode(const nlohmann::json &params);

    /**
     * @brief 获取当前跟踪速度
     * @return 当前跟踪速度
     */
    virtual std::string getTrackingSpeed(const nlohmann::json &params);

    /**
     * @brief 将望远镜回到家位置
     * @return 是否成功将望远镜回到家位置
     */
    virtual bool Home(const nlohmann::json &params);

    /**
     * @brief 判断望远镜是否在家位置
     * @return 返回 true 表示望远镜在家位置，否则返回 false
     */
    virtual bool isAtHome(const nlohmann::json &params);

    /**
     * @brief 设置家位置
     * @return 是否成功设置家位置
     */
    virtual bool setHomePosition(const nlohmann::json &params);

    /**
     * @brief 获取望远镜是否可以回到家位置
     * @return 返回 true 表示望远镜可以回到家位置，否则返回 false
     */
    virtual bool isHomeAvailable(const nlohmann::json &params);

    /**
     * @brief 停车
     * @return 是否成功停车
     */
    virtual bool Park(const nlohmann::json &params);

    /**
     * @brief 解除停车状态
     * @return 是否成功解除停车状态
     */
    virtual bool Unpark(const nlohmann::json &params);

    /**
     * @brief 判断望远镜是否在停车位置
     * @return 返回 true 表示位于停车位置，否则返回 false
     */
    virtual bool isAtPark(const nlohmann::json &params);

    /**
     * @brief 设置停车位置
     * @return 是否成功设置停车位置
     */
    virtual bool setParkPosition(const nlohmann::json &params);

    /**
     * @brief 获取望远镜是否可以停车
     * @return 返回 true 表示望远镜可以停车，否则返回 false
     */
    virtual bool isParkAvailable(const nlohmann::json &params);
};