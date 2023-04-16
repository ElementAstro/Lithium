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

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    class INDIFocuser : public Focuser, public OpenAptIndiClient
    {
        // INDI Parameters
    private:
        ISwitchVectorProperty *connection_prop;
        ISwitchVectorProperty *mode_prop;              // Focuser mode , absolute or relative
        ISwitchVectorProperty *motion_prop;            // Focuser motion , inward or outward
        INumberVectorProperty *speed_prop;             // Focuser speed , default is 1
        INumberVectorProperty *absolute_position_prop; // Focuser absolute position
        INumberVectorProperty *relative_position_prop; // Focuser relative position
        INumberVectorProperty *max_position_prop;      // Focuser max position
        INumberVectorProperty *temperature_prop;       // Focuser temperature
        ISwitchVectorProperty *rate_prop;
        INumberVectorProperty *delay_prop;
        ISwitchVectorProperty *backlash_prop;
        INumber *indi_max_position;
        INumber *indi_focuser_temperature;
        INumberVectorProperty *focuserinfo_prop;
        ITextVectorProperty *focuser_port;
        INDI::BaseDevice *focuser_device;

        bool is_ready;
        bool has_blob;

        std::string indi_focuser_port = "";
        std::string indi_focuser_rate = "";

        std::string indi_focuser_cmd;
        std::string indi_focuser_exec = "";
        std::string indi_focuser_version = "";
        std::string indi_focuser_interface = "";

    public:
        INDIFocuser(const std::string &name);
        ~INDIFocuser();

        bool connect(std::string name) override;
        bool disconnect() override;
        bool reconnect() override;
        bool scanForAvailableDevices() override;

        virtual bool moveTo(const int position) override;
        virtual bool moveToAbsolute(const int position) override;
        virtual bool moveStep(const int step) override;
        virtual bool moveStepAbsolute(const int step) override;

        virtual bool AbortMove() override;

        virtual bool setMaxPosition(int max_position) override;

        virtual double getTemperature() override;

        virtual bool haveBacklash() override;
        virtual bool setBacklash(int value) override;

        // 获取简单任务
        std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) override;
        // 获取条件任务
        std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) override;
        // 获取循环任务
        std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) override;

    protected:
        void ClearStatus();

    protected:
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