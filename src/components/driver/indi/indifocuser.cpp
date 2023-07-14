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

namespace Lithium
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
        spdlog::debug("{} Received message: {}", _name, dp->messageQueue(messageID));
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
        spdlog::debug("{} Received BLOB {} len = {} size = {}", _name, bp->name, bp->bloblen, bp->size);
    }

    void INDIFocuser::newProperty(INDI::Property *property)
    {
        std::string PropName(property->getName());
        INDI_PROPERTY_TYPE Proptype = property->getType();

        if (Proptype != INDI_TEXT && Proptype != INDI_SWITCH && Proptype != INDI_NUMBER)
        {
            spdlog::warn("{} Unknown property type: {}", _name, Proptype);
            return;
        }

        if (PropName.empty())
        {
            spdlog::warn("{} Property name is empty", _name);
            return;
        }

        bool switch_on = false;
        if (Proptype == INDI_SWITCH)
        {
            ISwitchVectorProperty *switch_prop = property->getSwitch();
            ISwitch *switch_connect = IUFindSwitch(switch_prop, "CONNECT");
            if (switch_connect)
            {
                switch_on = (switch_connect->s == ISS_ON);
            }
        }

        double prop_value = 0;
        if (Proptype == INDI_NUMBER)
        {
            INumberVectorProperty *num_prop = property->getNumber();
            INumber *num_value = IUFindNumber(num_prop, "FOCUS_ABSOLUTE_POSITION");
            if (num_value)
            {
                prop_value = num_value->value;
            }
            else
            {
                spdlog::warn("{} Unknown number property: {}", _name, PropName);
                return;
            }
        }

        switch (property->getType())
        {
        case INDI_TEXT:
        {
            if (PropName == "DEVICE_PORT")
            {
                spdlog::debug("{} Found device port for {}", _name, property->getDeviceName());
                focuser_port = property->getText();
            }
            else if (PropName == "DRIVER_INFO")
            {
                device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
                indi_focuser_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
                indi_focuser_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
                indi_focuser_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
                focuser_info["driver"]["name"] = device_name;
                focuser_info["driver"]["exec"] = indi_focuser_exec;
                focuser_info["driver"]["version"] = indi_focuser_version;
                focuser_info["driver"]["interfaces"] = indi_focuser_interface;
                spdlog::debug("{} Name : {} connected exec {}", _name, device_name, indi_focuser_exec);
            }
            else if (PropName == indi_focuser_cmd + "DEVICE_PORT")
            {
                indi_focuser_port = IUFindText(property->getText(), "PORT")->text;
                spdlog::debug("{} USB Port : {}", _name, indi_focuser_port);
            }
            break;
        }
        case INDI_SWITCH:
        {
            if (PropName == "CONNECTION")
            {
                spdlog::debug("{} Found CONNECTION for {} {}", _name, property->getDeviceName(), PropName);
                connection_prop = property->getSwitch();
                is_connected = switch_on;
                if (!is_connected)
                {
                    connection_prop->sp->s = ISS_ON;
                    sendNewSwitch(connection_prop);
                }
                spdlog::debug("{} Connected {}", _name, is_connected);
            }
            else if (PropName == indi_focuser_cmd + "Mode")
            {
                mode_prop = property->getSwitch();
                newSwitch(mode_prop);
            }
            else if (PropName == indi_focuser_cmd + "DEVICE_BAUD_RATE")
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
                spdlog::debug("{} baud rate : {}", _name, indi_focuser_rate);
            }
            else if (PropName == indi_focuser_cmd + "FOCUS_MOTION")
            {
                current_motion = (IUFindSwitch(motion_prop, "FOCUS_INWARD")->s == ISS_ON) ? 0 : 1;
                spdlog::debug("{} is moving {}", _name, current_motion ? "outward" : "inward");
            }
            else if (PropName == indi_focuser_cmd + "FOCUS_BACKLASH_TOGGLE")
            {
                has_backlash = (IUFindSwitch(backlash_prop, "INDI_ENABLED")->s == ISS_ON);
                spdlog::debug("{} Has Backlash : {}", _name, has_backlash);
            }
            break;
        }
        case INDI_NUMBER:
        {
            if (PropName == indi_focuser_cmd + "INFO")
            {
                focuserinfo_prop = property->getNumber();
                newNumber(focuserinfo_prop);
            }
            else if (PropName == indi_focuser_cmd + "FOCUS_SPEED")
            {
                speed_prop = property->getNumber();
                current_speed = prop_value;
                spdlog::debug("{} Current Speed : {}", _name, current_speed);
            }
            else if (PropName == indi_focuser_cmd + "ABS_FOCUS_POSITION")
            {
                absolute_position_prop = property->getNumber();
                current_position = prop_value;
                spdlog::debug("{} Current Absolute Position : {}", _name, current_position);
            }
            else if (PropName == indi_focuser_cmd + "DELAY")
            {
                delay_prop = property->getNumber();
                delay = prop_value;
                spdlog::debug("{} Current Delay : {}", _name, delay);
            }
            else if (PropName == indi_focuser_cmd + "FOCUS_TEMPERATURE")
            {
                temperature_prop = property->getNumber();
                current_temperature = prop_value;
                spdlog::debug("{} Current Temperature : {}", _name, current_temperature);
            }
            else if (PropName == indi_focuser_cmd + "FOCUS_MAX")
            {
                max_position_prop = property->getNumber();
                max_position = prop_value;
                spdlog::debug("{} Max Position : {}", _name, max_position);
            }
            else
            {
                spdlog::warn("{} Unknown number property: {}", _name, PropName);
            }
            break;
        }
        default:
        {
            spdlog::warn("{} Unknown property type: {}", _name, Proptype);
            break;
        }
        }
    }

    void INDIFocuser::IndiServerConnected()
    {
        spdlog::debug("{} connection succeeded", _name);
        is_connected = true;
    }

    void INDIFocuser::IndiServerDisconnected(int exit_code)
    {
        spdlog::debug("{}: serverDisconnected", _name);
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            spdlog::debug("{} : INDI server disconnected", _name);
    }

    void INDIFocuser::removeDevice(INDI::BaseDevice *dp)
    {
        ClearStatus();
        spdlog::info("{} disconnected", _name);
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
        spdlog::debug("INDI Focuser {} init successfully", name);
    }

    INDIFocuser::~INDIFocuser()
    {
    }

    bool INDIFocuser::connect(std::string name)
    {
        spdlog::debug("Trying to connect to {}", name);
        if (is_connected)
        {
            spdlog::warn("{} is already connected", _name);
            return true;
        }
        setServer(hostname.c_str(), port);
        // Receive messages only for our focuser.
        watchDevice(name.c_str());
        // Connect to server.
        if (connectServer())
        {
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
        if (!is_connected)
        {
            spdlog::error("Focuser is not connected");
            return false;
        }
        if (absolute_position_prop == nullptr)
        {
            spdlog::error("absolute_position_prop is null");
            return false;
        }
        if (position > max_position)
        {
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
        if (temperature_prop == nullptr)
        {
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

    std::shared_ptr<Lithium::SimpleTask> INDIFocuser::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
    {
        if (task_name == "MoveToAbsolute")
        {
            spdlog::debug("MoveToAbsolute task with parameters: {}", params.dump());
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->moveToAbsolute(tpramas["position"].get<int>());
                },
                {params}));
        }
        else if (task_name == "MoveStepAbsolute")
        {
            spdlog::debug("MoveStepAbsolute task with parameters: {}", params.dump());
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->moveStepAbsolute(tpramas["step"].get<int>());
                },
                {params}));
        }
        else if (task_name == "AbortMove")
        {
            spdlog::debug("AbortMove task");
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &)
                {
                    this->AbortMove();
                },
                {params}));
        }
        else if (task_name == "GetMaxPosition")
        {
            spdlog::debug("GetMaxPosition task");
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &)
                {
                    this->getMaxPosition();
                },
                {params}));
        }
        else if (task_name == "SetMaxPosition")
        {
            spdlog::debug("SetMaxPosition task with parameters: {}", params.dump());
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->setMaxPosition(tpramas["max_position"].get<int>());
                },
                {params}));
        }
        else if (task_name == "HaveBacklash")
        {
            spdlog::debug("HaveBacklash task");
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &)
                {
                    this->haveBacklash();
                },
                {params}));
        }
        else if (task_name == "SetBacklash")
        {
            spdlog::debug("SetBacklash task with parameters: {}", params.dump());
            return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(
                [this](const nlohmann::json &tpramas)
                {
                    this->setBacklash(tpramas["backlash"].get<int>());
                },
                {params}));
        }
        spdlog::error("Unknown type of the INDI Focuser task: {}", task_name);
        return nullptr;
    }

    std::shared_ptr<Lithium::ConditionalTask> INDIFocuser::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getCondtionalTask function not implemented");
        return nullptr;
    }

    std::shared_ptr<Lithium::LoopTask> INDIFocuser::getLoopTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getLoopTask function not implemented");
        return nullptr;
    }
} // namespace Lithium
