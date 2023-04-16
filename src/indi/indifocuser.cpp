/*
 * indifocuser.cpp
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

Description: {}

**************************************************/

#include "indifocuser.hpp"

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    void INDIFocuser::newDevice(INDI::BaseDevice *dp)
    {
        if (dp->getDeviceName() == device_name)
        {
            focuser_device = dp;
        }
    }

    void INDIFocuser::newSwitch(ISwitchVectorProperty *svp)
    {
        std::string const connection_str{"CONNECTION"};
        std::string const mode_str{"Mode"};
        std::string const baud_rate_str{"DEVICE_BAUD_RATE"};

        if (svp->name == connection_str)
        {
            ISwitch *connectswitch = IUFindSwitch(svp, "CONNECT");
            if (connectswitch->s == ISS_ON)
            {
                is_connected = true;
                spdlog::info("{} is connected", _name);
            }
            else
            {
                if (is_ready)
                {
                    ClearStatus();
                    spdlog::info("{} is disconnected", _name);
                }
            }
        }
        else if (svp->name == mode_str)
        {
            ISwitch *modeswitch = IUFindSwitch(svp, "All");
            if (modeswitch->s == ISS_ON)
            {
                can_absolute_move = true;
                current_mode = 0;
            }
            else
            {
                modeswitch = IUFindSwitch(svp, "Absolute");
                if (modeswitch->s == ISS_ON)
                {
                    can_absolute_move = true;
                    current_mode = 1;
                }
                else
                {
                    can_absolute_move = false;
                    current_mode = 2;
                }
            }
        }
        else if (svp->name == baud_rate_str)
        {
            std::string const baud_9600{"9600"};
            std::string const baud_19200{"19200"};
            std::string const baud_38400{"38400"};
            std::string const baud_57600{"57600"};
            std::string const baud_115200{"115200"};
            std::string const baud_230400{"230400"};

            if (IUFindSwitch(svp, "9600")->s == ISS_ON)
                indi_focuser_rate = baud_9600;
            else if (IUFindSwitch(svp, "19200")->s == ISS_ON)
                indi_focuser_rate = baud_19200;
            else if (IUFindSwitch(svp, "38400")->s == ISS_ON)
                indi_focuser_rate = baud_38400;
            else if (IUFindSwitch(svp, "57600")->s == ISS_ON)
                indi_focuser_rate = baud_57600;
            else if (IUFindSwitch(svp, "115200")->s == ISS_ON)
                indi_focuser_rate = baud_115200;
            else if (IUFindSwitch(svp, "230400")->s == ISS_ON)
                indi_focuser_rate = baud_230400;

            spdlog::debug("{} baud rate : {}", _name, indi_focuser_rate);
        }
    }

    void INDIFocuser::newMessage(INDI::BaseDevice *dp, int messageID)
    {
        spdlog::debug("{} Received message: {}",_name, dp->messageQueue(messageID));
    }

    inline static const char *StateStr(IPState st)
    {
        switch (st)
        {
            default:
            case IPS_IDLE:
                return "Idle";
            case IPS_OK:
                return "Ok";
            case IPS_BUSY:
                return "Busy";
            case IPS_ALERT:
                return "Alert";
        }
    }

    void INDIFocuser::newNumber(INumberVectorProperty *nvp)
    {
        std::ostringstream os;
        for (int i = 0; i < nvp->nnp; i++)
        {
            if (i)
                os << ',';
            os << nvp->np[i].name << ':' << nvp->np[i].value;
        }
        spdlog::debug("{} Received Number: {} = {} state = {}", _name, nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == focuserinfo_prop)
        {
            // do nothing
        }
        else if (nvp == temperature_prop)
        {
            current_temperature = IUFindNumber(temperature_prop, "CCD_TEMPERATURE_VALUE")->value;
        }
    }

    void INDIFocuser::newText(ITextVectorProperty *tvp)
    {
        spdlog::debug("{} Received Text: {} = {}", _name, tvp->name, tvp->tp->text);
    }


    void INDIFocuser::newBLOB(IBLOB *bp)
    {
        // we go here every time a new blob is available
        // this is normally the image from the Focuser
        spdlog::debug("{} Received BLOB {} len = {} size = {}",_name, bp->name, bp->bloblen, bp->size);
    }

    void INDIFocuser::newProperty(INDI::Property *property)
    {
        std::string PropName(property->getName());
        INDI_PROPERTY_TYPE Proptype = property->getType();

        spdlog::debug("{} Property: {}",_name, property->getName());

        if (PropName == "DEVICE_PORT" && Proptype == INDI_TEXT)
        {
            spdlog::debug("{} Found device port for {} ",_name, property->getDeviceName());
            focuser_port = property->getText();
        }
        else if (PropName == "CONNECTION" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("{} Found CONNECTION for {} {}",_name, property->getDeviceName(), PropName);
            connection_prop = property->getSwitch();
            ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
            is_connected = (connectswitch->s == ISS_ON);
            if (!is_connected)
            {
                connection_prop->sp->s = ISS_ON;
                sendNewSwitch(connection_prop);
            }
            spdlog::debug("{} Connected {}",_name, is_connected);
        }
        else if (PropName == "DRIVER_INFO" && Proptype == INDI_TEXT)
        {
            device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
            indi_focuser_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
            indi_focuser_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
            indi_focuser_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
            spdlog::debug("{} Name : {} connected exec {}",_name, device_name, indi_focuser_exec);
        }
        else if (PropName == indi_focuser_cmd + "INFO" && Proptype == INDI_NUMBER)
        {
            focuserinfo_prop = property->getNumber();
            newNumber(focuserinfo_prop);
        }
        else if (PropName == indi_focuser_cmd + "Mode" && Proptype == INDI_SWITCH)
        {
            mode_prop = property->getSwitch();
            newSwitch(mode_prop);
        }
        else if (PropName == indi_focuser_cmd + "DEVICE_BAUD_RATE" && Proptype == INDI_SWITCH)
        {
            rate_prop = property->getSwitch();
            if (IUFindSwitch(rate_prop, "9600")->s == ISS_ON)
                indi_focuser_rate = "9600";
            else if (IUFindSwitch(rate_prop, "19200")->s == ISS_ON)
                indi_focuser_rate = "19200";
            else if (IUFindSwitch(rate_prop, "38400")->s == ISS_ON)
                indi_focuser_rate = "38400";
            else if (IUFindSwitch(rate_prop, "57600")->s == ISS_ON)
                indi_focuser_rate = "57600";
            else if (IUFindSwitch(rate_prop, "115200")->s == ISS_ON)
                indi_focuser_rate = "115200";
            else if (IUFindSwitch(rate_prop, "230400")->s == ISS_ON)
                indi_focuser_rate = "230400";
            spdlog::debug("{} baud rate : {}",_name, indi_focuser_rate);
        }
        else if (PropName == indi_focuser_cmd + "DEVICE_PORT" && Proptype == INDI_TEXT)
        {
            indi_focuser_port = IUFindText(property->getText(), "PORT")->text;
            spdlog::debug("{} USB Port : {}",_name, indi_focuser_port);
        }
        else if (PropName == indi_focuser_cmd + "FOCUS_MOTION" && Proptype == INDI_SWITCH)
        {
            motion_prop = property->getSwitch();
            if (IUFindSwitch(motion_prop, "FOCUS_INWARD")->s == ISS_ON)
            {
                current_motion = 0;
                spdlog::debug("{} is moving inward",_name);
            }
            else
            {
                current_motion = 1;
                spdlog::debug("{} is moving outward",_name);
            }
        }
        else if (PropName == indi_focuser_cmd + "FOCUS_SPEED" && Proptype == INDI_NUMBER)
        {
            speed_prop = property->getNumber();
            current_speed = IUFindNumber(speed_prop, "FOCUS_SPEED_VALUE")->value;
            spdlog::debug("{} Current Speed : {}",_name, current_speed);
        }
        else if (PropName == indi_focuser_cmd + "ABS_FOCUS_POSITION" && Proptype == INDI_NUMBER)
        {
            absolute_position_prop = property->getNumber();
            current_position = IUFindNumber(absolute_position_prop, "FOCUS_ABSOLUTE_POSITION")->value;
            spdlog::debug("{} Current Absolute Position : {}",_name, current_position);
        }
        else if (PropName == indi_focuser_cmd + "DELAY" && Proptype == INDI_NUMBER)
        {
            delay_prop = property->getNumber();
            delay = IUFindNumber(delay_prop, "DELAY_VALUE")->value;
            spdlog::debug("{} Current Delay : {}",_name, delay);
        }
        else if (PropName == indi_focuser_cmd + "FOCUS_TEMPERATURE" && Proptype == INDI_NUMBER)
        {
            temperature_prop = property->getNumber();
            current_temperature = IUFindNumber(temperature_prop, "TEMPERATURE")->value;
            spdlog::debug("{} Current Temperature : {}",_name, current_temperature);
        }
        else if (PropName == indi_focuser_cmd + "FOCUS_BACKLASH_TOGGLE" && Proptype == INDI_SWITCH)
        {
            backlash_prop = property->getSwitch();
            has_backlash = IUFindSwitch(backlash_prop, "INDI_ENABLED")->s == ISS_ON;
            spdlog::debug("{} Has Backlash : {}",_name, has_backlash);
        }
        else if (PropName == indi_focuser_cmd + "FOCUS_MAX" && Proptype == INDI_NUMBER)
        {
            max_position_prop = property->getNumber();
            max_position = IUFindNumber(max_position_prop, "FOCUS_MAX_VALUE")->value;
            spdlog::debug("{} Max Position : {}",_name, max_position);
        }
    }

    void INDIFocuser::IndiServerConnected()
    {
        spdlog::debug("{} connection succeeded",_name);
        is_connected = true;
    }

    void INDIFocuser::IndiServerDisconnected(int exit_code)
    {
        spdlog::debug("{}: serverDisconnected",_name);
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            spdlog::debug("{} : INDI server disconnected",_name);
    }

    void INDIFocuser::removeDevice(INDI::BaseDevice *dp)
    {
        ClearStatus();
        spdlog::info("{} disconnected",_name);
    }

    void INDIFocuser::ClearStatus()
    {
        connection_prop = nullptr;
        focuser_port = nullptr;
        focuser_device = nullptr;
        connection_prop = nullptr;
        mode_prop = nullptr;              // Focuser mode , absolute or relative
        motion_prop = nullptr;            // Focuser motion , inward or outward
        speed_prop = nullptr;             // Focuser speed , default is 1
        absolute_position_prop = nullptr; // Focuser absolute position
        relative_position_prop = nullptr; // Focuser relative position
        max_position_prop = nullptr;      // Focuser max position
        temperature_prop = nullptr;       // Focuser temperature
        rate_prop = nullptr;
        delay_prop = nullptr;
        backlash_prop = nullptr;
        indi_max_position = nullptr;
        indi_focuser_temperature = nullptr;
        focuserinfo_prop = nullptr;
        focuser_port = nullptr;
        focuser_device = nullptr;
    }

    INDIFocuser::INDIFocuser(const std::string &name) : Focuser(name)
    {
        spdlog::debug("INDI Focuser {} init successfully",name);
    }

    INDIFocuser::~INDIFocuser()
    {
    }

    bool INDIFocuser::connect(std::string name)
    {
        spdlog::debug("Trying to connect to {}", name);
        if (is_connected) {
            spdlog::warn("{} is already connected", _name);
            return true;
        }
        setServer(hostname.c_str(), port);
        // Receive messages only for our camera.
        watchDevice(name.c_str());
        // Connect to server.
        if (connectServer()) {
            spdlog::debug("{}: connectServer done ready = {}", _name, is_ready);
            connectDevice(name.c_str());
            is_connected = true;
            return true;
        }
        is_connected = false;
        return false;
    }

    bool INDIFocuser::disconnect()
    {
        return true;
    }

    bool INDIFocuser::reconnect()
    {
        disconnect();
        return connect(_name);
    }

    bool INDIFocuser::scanForAvailableDevices()
    {
        spdlog::warn("scanForAvailableDevices function not implemented");
        return false;
    }

    bool INDIFocuser::moveTo(const int position)
    {
        return moveToAbsolute(position);
    }

    bool INDIFocuser::moveToAbsolute(const int position)
    {
        if (!is_connected) {
            spdlog::error("Focuser is not connected");
            return false;
        }
        if (absolute_position_prop == nullptr) {
            spdlog::error("absolute_position_prop is null");
            return false;
        }
        if (position > max_position) {
            spdlog::error("Position is out of the right range");
            return false;
        }
        absolute_position_prop->np->value = position;
        sendNewNumber(absolute_position_prop);
        return true;
    }

    bool INDIFocuser::moveStep(const int step)
    {
        return moveStepAbsolute(step);
    }

    bool INDIFocuser::moveStepAbsolute(const int step)
    {
        spdlog::warn("moveStepAbsolute function not implemented");
        return false;
    }

    bool INDIFocuser::AbortMove()
    {
        spdlog::warn("AbortMove function not implemented");
        return false;
    }

    bool INDIFocuser::setMaxPosition(int max_position)
    {
        spdlog::warn("setMaxPosition function not implemented");
        return false;
    }

    double INDIFocuser::getTemperature()
    {
        if (temperature_prop == nullptr) {
            spdlog::error("temperature_prop is null");
            return -1;
        }
        return temperature_prop->np->value;
    }

    bool INDIFocuser::haveBacklash()
    {
        spdlog::warn("haveBacklash function not implemented");
        return false;
    }

    bool INDIFocuser::setBacklash(int value)
    {
        spdlog::warn("setBacklash function not implemented");
        return false;
    }

    std::shared_ptr<OpenAPT::SimpleTask> INDIFocuser::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
    {
        if (task_name == "MoveToAbsolute")
        {
            spdlog::debug("MoveToAbsolute task with parameters: {}", params.dump());
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->moveToAbsolute(tpramas["position"].get<int>());
                },
                {params}));
        }
        else if (task_name == "MoveStepAbsolute")
        {
            spdlog::debug("MoveStepAbsolute task with parameters: {}", params.dump());
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->moveStepAbsolute(tpramas["step"].get<int>());
                },
                {params}));
        }
        else if (task_name == "AbortMove")
        {
            spdlog::debug("AbortMove task");
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &)
                {
                    this->AbortMove();
                },
                {params}));
        }
        else if (task_name == "GetMaxPosition")
        {
            spdlog::debug("GetMaxPosition task");
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &)
                {
                    this->getMaxPosition();
                },
                {params}));
        }
        else if (task_name == "SetMaxPosition")
        {
            spdlog::debug("SetMaxPosition task with parameters: {}", params.dump());
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->setMaxPosition(tpramas["max_position"].get<int>());
                },
                {params}));
        }
        else if (task_name == "HaveBacklash")
        {
            spdlog::debug("HaveBacklash task");
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &)
                {
                    this->haveBacklash();
                },
                {params}));
        }
        else if (task_name == "SetBacklash")
        {
            spdlog::debug("SetBacklash task with parameters: {}", params.dump());
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->setBacklash(tpramas["backlash"].get<int>());
                },
                {params}));
        }
        spdlog::error("Unknown type of the INDI Focuser task: {}", task_name);
        return nullptr;
    }

    std::shared_ptr<OpenAPT::ConditionalTask> INDIFocuser::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getCondtionalTask function not implemented");
        return nullptr;
    }

    std::shared_ptr<OpenAPT::LoopTask> INDIFocuser::getLoopTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getLoopTask function not implemented");
        return nullptr;
    }
} // namespace OpenAPT
