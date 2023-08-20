/*
 * indifocuser.hpp
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

Date: 2023-4-10

Description: INDI Focuser

**************************************************/

#pragma once

#include "api/indiclient.hpp"
#include "device/basic_device.hpp"

#include <libindi/basedevice.h>
#include <libindi/indiproperty.h>

#include <string>

#include "nlohmann/json.hpp"

namespace Lithium
{
    class INDIFocuser : public Focuser, public LithiumIndiClient
    {
        // INDI 客户端参数
        ISwitchVectorProperty *connection_prop;        // 连接属性指针
        ISwitchVectorProperty *mode_prop;              // 焦距器模式（绝对或相对）属性指针
        ISwitchVectorProperty *motion_prop;            // 焦距器运动方向（向内或向外）属性指针
        INumberVectorProperty *speed_prop;             // 焦距器速度属性指针，默认为 1
        INumberVectorProperty *absolute_position_prop; // 焦距器绝对位置属性指针
        INumberVectorProperty *relative_position_prop; // 焦距器相对位置属性指针
        INumberVectorProperty *max_position_prop;      // 焦距器最大位置属性指针
        INumberVectorProperty *temperature_prop;       // 焦距器温度属性指针
        ISwitchVectorProperty *rate_prop;              // 焦距器速率属性指针
        INumberVectorProperty *delay_prop;             // 焦距器延迟属性指针
        ISwitchVectorProperty *backlash_prop;          // 焦距器反向间隙属性指针
        INumber *indi_max_position;                    // 焦距器 indi 最大位置属性指针
        INumber *indi_focuser_temperature;             // 焦距器 indi 温度属性指针
        INumberVectorProperty *focuserinfo_prop;       // 焦距器用户信息属性指针
        ITextVectorProperty *focuser_port;             // 焦距器端口属性指针
        LITHIUM::BaseDevice *focuser_device;              // 焦距器设备指针

        bool is_ready; // 是否连接成功标志
        bool has_blob; // 是否接收 blob 数据标志

        std::string indi_focuser_port = ""; // 焦距器所选端口
        std::string indi_focuser_rate = ""; // 焦距器所选速率

        std::string indi_focuser_cmd;            // INDI 命令字符串
        std::string indi_focuser_exec = "";      // INDI 设备执行文件路径
        std::string indi_focuser_version = "";   // INDI 设备固件版本
        std::string indi_focuser_interface = ""; // INDI 接口版本

        nlohmann::json focuser_json;

    public:
        /**
         * @brief 构造函数，初始化 INDIFocuser 类
         *
         * @param name 焦距器名字
         */
        INDIFocuser(const std::string &name);

        /**
         * @brief 析构函数，释放 INDIFocuser 类相关资源
         *
         */
        ~INDIFocuser();

        /**
         * @brief 连接焦距器
         *
         * @param name 焦距器名字
         * @return true 连接成功
         * @return false 连接失败
         */
        bool connect(std::string name) override;

        /**
         * @brief 断开连接
         *
         * @return true 断开成功
         * @return false 断开失败
         */
        bool disconnect() override;

        /**
         * @brief 重新连接
         *
         * @return true 重新连接成功
         * @return false 重新连接失败
         */
        bool reconnect() override;

        /**
         * @brief 搜索可用设备
         *
         * @return true 搜索成功
         * @return false 搜索失败
         */
        bool scanForAvailableDevices() override;

        /**
         * @brief 移动焦距器到指定位置（相对位置）
         *
         * @param position 相对位置
         * @return true 移动成功
         * @return false 移动失败
         */
        virtual bool moveTo(const int position) override;

        /**
         * @brief 移动焦距器到指定位置（绝对位置）
         *
         * @param position 绝对位置
         * @return true 移动成功
         * @return false 移动失败
         */
        virtual bool moveToAbsolute(const int position) override;

        /**
         * @brief 移动焦距器指定步数（相对位置）
         *
         * @param step 步数
         * @return true 移动成功
         * @return false 移动失败
         */
        virtual bool moveStep(const int step) override;

        /**
         * @brief 移动焦距器指定步数（绝对位置）
         *
         * @param step 步数
         * @return true 移动成功
         * @return false 移动失败
         */
        virtual bool moveStepAbsolute(const int step) override;

        /**
         * @brief 中止移动焦距器
         *
         * @return true 中止成功
         * @return false 中止失败
         */
        virtual bool AbortMove() override;

        /**
         * @brief 设置最大位置
         *
         * @param max_position 最大位置
         * @return true 设置成功
         * @return false 设置失败
         */
        virtual bool setMaxPosition(int max_position) override;

        /**
         * @brief 获取焦距器温度
         *
         * @return double 焦距器温度
         */
        virtual double getTemperature() override;

        /**
         * @brief 是否有反向间隙
         *
         * @return true 有反向间隙
         * @return false 没有反向间隙
         */
        virtual bool haveBacklash() override;

        /**
         * @brief 设置反向间隙
         *
         * @param value 反向间隙值
         * @return true 设置成功
         * @return false 设置失败
         */
        virtual bool setBacklash(int value) override;

        /**
         * @brief 获取简单任务
         *
         * @param task_name 任务名字
         * @param params 任务参数
         * @return std::shared_ptr<Lithium::SimpleTask> 简单任务指针
         */
        std::shared_ptr<Lithium::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) override;

        /**
         * @brief 获取条件任务
         *
         * @param task_name 任务名字
         * @param params 任务参数
         * @return std::shared_ptr<Lithium::ConditionalTask> 条件任务指针
         */
        std::shared_ptr<Lithium::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) override;

        /**
         * @brief 获取循环任务
         *
         * @param task_name 任务名字
         * @param params 任务参数
         * @return std::shared_ptr<Lithium::LoopTask> 循环任务指针
         */
        std::shared_ptr<Lithium::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) override;

    protected:
        /**
         * @brief 清除状态
         *
         */
        void ClearStatus();

    protected:
        void newDevice(LITHIUM::BaseDevice *dp) override;
        void removeDevice(LITHIUM::BaseDevice *dp) override;
        void newProperty(LITHIUM::Property *property) override;
        void removeProperty(LITHIUM::Property *property) override {}
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;
        void newNumber(INumberVectorProperty *nvp) override;
        void newMessage(LITHIUM::BaseDevice *dp, int messageID) override;
        void newText(ITextVectorProperty *tvp) override;
        void newLight(ILightVectorProperty *lvp) override {}
        void IndiServerConnected() override;
        void IndiServerDisconnected(int exit_code) override;
    };

}