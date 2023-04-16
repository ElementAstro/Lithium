/*
 * indiTelescope.hpp
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

Description: INDI Telescope

**************************************************/

#pragma once

#include "api/indiclient.hpp"
#include "device/basic_device.hpp"

#include <libindi/basedevice.h>
#include <libindi/indiproperty.h>

#include <string>

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    class INDITelescope : public Telescope, public OpenAptIndiClient
    {
    private:
        // INDI 客户端参数
        ISwitchVectorProperty *connection_prop;    // 连接属性指针
        INumberVectorProperty *telescopeinfo_prop; // 望远镜信息属性指针
        ITextVectorProperty *telescope_port;       // 望远镜端口属性指针
        ISwitchVectorProperty *rate_prop;          // 望远镜速率属性指针
        INDI::BaseDevice *telescope_device;        // 望远镜设备指针

        bool is_ready; // 是否连接成功标志
        bool has_blob; // 是否接收 blob 数据标志

        std::string indi_telescope_port = ""; // 望远镜所选端口
        std::string indi_telescope_rate = ""; // 望远镜所选速率

        std::string indi_telescope_cmd;            // INDI 命令字符串
        std::string indi_telescope_exec = "";      // INDI 设备执行文件路径
        std::string indi_telescope_version = "";   // INDI 设备固件版本
        std::string indi_telescope_interface = ""; // INDI 接口版本

    public:
        /**
         * @brief 构造函数
         *
         * @param name 望远镜名字
         */
        INDITelescope(const std::string &name);

        /**
         * @brief 析构函数
         *
         */
        ~INDITelescope();

        /**
         * @brief 连接望远镜
         *
         * @param name 望远镜名字
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
         * @brief 将望远镜移动到指定位置
         *
         * @param ra 赤经
         * @param dec 赤纬
         * @param j2000 是否使用 J2000 坐标系
         * @return true 移动成功
         * @return false 移动失败
         */
        bool SlewTo(const std::string &ra, const std::string &dec, const bool j2000 = false) override;

        /**
         * @brief 中止望远镜命令
         *
         * @return true 中止成功
         * @return false 中止失败
         */
        bool Abort() override;

        /**
         * @brief 开始追踪指定的模型和速率
         *
         * @param model 模型名称
         * @param speed 速率
         * @return true 开始追踪成功
         * @return false 开始追踪失败
         */
        bool StartTracking(const std::string &model, const std::string &speed) override;

        /**
         * @brief 停止追踪
         *
         * @return true 停止追踪成功
         * @return false 停止追踪失败
         */
        bool StopTracking() override;

        /**
         * @brief 设置望远镜追踪模式
         *
         * @param mode 追踪模式
         * @return true 设置成功
         * @return false 设置失败
         */
        bool setTrackingMode(const std::string &mode) override;

        /**
         * @brief 设置望远镜追踪速率
         *
         * @param speed 速率
         * @return true 设置成功
         * @return false 设置失败
         */
        bool setTrackingSpeed(const std::string &speed) override;

        /**
         * @brief 回归原点
         *
         * @return true 回归成功
         * @return false 回归失败
         */
        bool Home() override;

        /**
         * @brief 是否在原点位置
         *
         * @return true 在原点位置
         * @return false 不在原点位置
         */
        bool isAtHome() override;

        /**
         * @brief 设置原点
         *
         * @return true 设置成功
         * @return false 设置失败
         */
        bool setHomePosition() override;

        /**
         * @brief 停泊望远镜
         *
         * @return true 停泊成功
         * @return false 停泊失败
         */
        bool Park() override;

        /**
         * @brief 解锁望远镜
         *
         * @return true 解锁成功
         * @return false 解锁失败
         */
        bool Unpark() override;

        /**
         * @brief 是否在停泊位置
         *
         * @return true 在停泊位置
         * @return false 不在停泊位置
         */
        bool isAtPark() override;

        /**
         * @brief 设置停泊位置
         *
         * @return true 设置成功
         * @return false 设置失败
         */
        bool setParkPosition() override;

        /**
         * @brief 获取简单任务
         *
         * @param task_name 任务名称
         * @param params 任务参数
         * @return std::shared_ptr<OpenAPT::SimpleTask> 简单任务指针
         */
        std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) override;

        /**
         * @brief 获取条件任务
         *
         * @param task_name 任务名称
         * @param params 任务参数
         * @return std::shared_ptr<OpenAPT::ConditionalTask> 条件任务指针
         */
        std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) override;

        /**
         * @brief 获取循环任务
         *
         * @param task_name 任务名称
         * @param params 任务参数
         * @return std::shared_ptr<OpenAPT::LoopTask> 循环任务指针
         */
        std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) override;

    protected:
        /**
         * @brief 清空状态
         *
         */
        void ClearStatus();

    protected:
        // INDI 回调函数
        void newDevice(INDI::BaseDevice *dp) override;
        void removeDevice(INDI::BaseDevice *dp) override;
        void newProperty(INDI::Property *property) override;
        void removeProperty(INDI::Property *property) override {}
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;
        void newNumber(INumberVectorProperty *nvp) override;
        void newMessage(INDI::BaseDevice *dp, int messageID) override;
        void newText(ITextVectorProperty *tvp) override;
        void newLight(ILightVectorProperty *lvp) override {}
        void IndiServerConnected() override;
        void IndiServerDisconnected(int exit_code) override;
    };

}