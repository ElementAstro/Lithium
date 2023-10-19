/*
 * indifilterwheel.cpp
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

Description: Hydrogen Filterwheel

**************************************************/

#include "hydrogenfilterwheel.hpp"

#include "config.h"

#include "loguru/loguru.hpp"

namespace Lithium
{
    void HydrogenFilterwheel::newDevice(HYDROGEN::BaseDevice *dp)
    {
        if (dp->getDeviceName() == device_name)
        {
            filter_device = dp;
        }
    }

    void HydrogenFilterwheel::newSwitch(ISwitchVectorProperty *svp)
    {
        std::string const connection_str{"CONNECTION"};
        std::string const baud_rate_str{"DEVICE_BAUD_RATE"};

        if (svp->name == connection_str)
        {
            ISwitch *connectswitch = IUFindSwitch(svp, "CONNECT");
            if (connectswitch->s == ISS_ON)
            {
                is_connected = true;
                DLOG_F(INFO, "{} is connected", _name);
            }
            else
            {
                if (is_ready)
                {
                    ClearStatus();
                    DLOG_F(INFO, "{} is disconnected", _name);
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
                indi_filter_rate = baud_9600;
            else if (IUFindSwitch(svp, "19200")->s == ISS_ON)
                indi_filter_rate = baud_19200;
            else if (IUFindSwitch(svp, "38400")->s == ISS_ON)
                indi_filter_rate = baud_38400;
            else if (IUFindSwitch(svp, "57600")->s == ISS_ON)
                indi_filter_rate = baud_57600;
            else if (IUFindSwitch(svp, "115200")->s == ISS_ON)
                indi_filter_rate = baud_115200;
            else if (IUFindSwitch(svp, "230400")->s == ISS_ON)
                indi_filter_rate = baud_230400;

            DLOG_F(INFO, "{} baud rate : {}", _name, indi_filter_rate);
        }
    }

    void HydrogenFilterwheel::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
    {
        DLOG_F(INFO, "{} Received message: {}", _name, dp->messageQueue(messageID));
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

    void HydrogenFilterwheel::newNumber(INumberVectorProperty *nvp)
    {
        std::ostringstream os;
        for (int i = 0; i < nvp->nnp; i++)
        {
            if (i)
                os << ',';
            os << nvp->np[i].name << ':' << nvp->np[i].value;
        }
        DLOG_F(INFO, "{} Received Number: {} = {} state = {}", _name, nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == filterinfo_prop)
        {
            // do nothing
        }
    }

    void HydrogenFilterwheel::newText(ITextVectorProperty *tvp)
    {
        DLOG_F(INFO, "{} Received Text: {} = {}", _name, tvp->name, tvp->tp->text);
    }

    void HydrogenFilterwheel::newBLOB(IBLOB *bp)
    {
        DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", _name, bp->name, bp->bloblen, bp->size);
    }

    void HydrogenFilterwheel::newProperty(HYDROGEN::Property *property)
    {
        std::string PropName(property->getName());
        HYDROGEN_PROPERTY_TYPE Proptype = property->getType();

        DLOG_F(INFO, "{} Property: {}", _name, property->getName());

        if (PropName == "DEVICE_PORT" && Proptype == HYDROGEN_TEXT)
        {
            DLOG_F(INFO, "{} Found device port for {} ", _name, property->getDeviceName());
            filter_port = property->getText();
        }
        else if (PropName == "CONNECTION" && Proptype == HYDROGEN_SWITCH)
        {
            DLOG_F(INFO, "{} Found CONNECTION for {} {}", _name, property->getDeviceName(), PropName);
            connection_prop = property->getSwitch();
            ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
            is_connected = (connectswitch->s == ISS_ON);
            if (!is_connected)
            {
                connection_prop->sp->s = ISS_ON;
                sendNewSwitch(connection_prop);
            }
            DLOG_F(INFO, "{} Connected {}", _name, is_connected);
        }
        else if (PropName == "DRIVER_INFO" && Proptype == HYDROGEN_TEXT)
        {
            device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
            indi_filter_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
            indi_filter_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
            indi_filter_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
            DLOG_F(INFO, "{} Name : {} connected exec {}", _name, device_name, indi_filter_exec);
        }
        else if (PropName == indi_filter_cmd + "INFO" && Proptype == HYDROGEN_NUMBER)
        {
            filterinfo_prop = property->getNumber();
            newNumber(filterinfo_prop);
        }
        else if (PropName == indi_filter_cmd + "DEVICE_BAUD_RATE" && Proptype == HYDROGEN_SWITCH)
        {
            rate_prop = property->getSwitch();
            if (IUFindSwitch(rate_prop, "9600")->s == ISS_ON)
                indi_filter_rate = "9600";
            else if (IUFindSwitch(rate_prop, "19200")->s == ISS_ON)
                indi_filter_rate = "19200";
            else if (IUFindSwitch(rate_prop, "38400")->s == ISS_ON)
                indi_filter_rate = "38400";
            else if (IUFindSwitch(rate_prop, "57600")->s == ISS_ON)
                indi_filter_rate = "57600";
            else if (IUFindSwitch(rate_prop, "115200")->s == ISS_ON)
                indi_filter_rate = "115200";
            else if (IUFindSwitch(rate_prop, "230400")->s == ISS_ON)
                indi_filter_rate = "230400";
            DLOG_F(INFO, "{} baud rate : {}", _name, indi_filter_rate);
        }
        else if (PropName == indi_filter_cmd + "DEVICE_PORT" && Proptype == HYDROGEN_TEXT)
        {
            indi_filter_port = IUFindText(property->getText(), "PORT")->text;
            DLOG_F(INFO, "{} USB Port : {}", _name, indi_filter_port);
        }
    }

    void HydrogenFilterwheel::IndiServerConnected()
    {
        DLOG_F(INFO, "{} connection succeeded", _name);
        is_connected = true;
    }

    void HydrogenFilterwheel::IndiServerDisconnected(int exit_code)
    {
        DLOG_F(INFO, "{}: serverDisconnected", _name);
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            DLOG_F(INFO, "{} : Hydrogen server disconnected", _name);
    }

    void HydrogenFilterwheel::removeDevice(HYDROGEN::BaseDevice *dp)
    {
        ClearStatus();
        DLOG_F(INFO, "{} disconnected", _name);
    }

    void HydrogenFilterwheel::ClearStatus()
    {
        connection_prop = nullptr;
        filter_port = nullptr;
        filter_device = nullptr;
        connection_prop = nullptr;
        rate_prop = nullptr;
        filterinfo_prop = nullptr;
        filter_port = nullptr;
        filter_device = nullptr;
    }

    HydrogenFilterwheel::HydrogenFilterwheel(const std::string &name) : Filterwheel(name)
    {
        DLOG_F(INFO, "Hydrogen filterwheel {} init successfully", name);
    }

    HydrogenFilterwheel::~HydrogenFilterwheel()
    {
    }

    bool HydrogenFilterwheel::connect(std::string name)
    {
        DLOG_F(INFO, "Trying to connect to {}", name);
        if (is_connected)
        {
            spdlog::warn("{} is already connected", _name);
            return true;
        }
        setServer(hostname.c_str(), port);
        // Receive messages only for our camera.
        watchDevice(name.c_str());
        // Connect to server.
        if (connectServer())
        {
            DLOG_F(INFO, "{}: connectServer done ready = {}", _name, is_ready);
            connectDevice(name.c_str());
            is_connected = true;
            return true;
        }
        is_connected = false;
        return false;
    }

    bool HydrogenFilterwheel::disconnect()
    {
        return true;
    }

    bool HydrogenFilterwheel::reconnect()
    {
        disconnect();
        return connect(_name);
    }

    bool HydrogenFilterwheel::scanForAvailableDevices()
    {
        spdlog::warn("scanForAvailableDevices function not implemented");
        return false;
    }

    std::shared_ptr<Lithium::SimpleTask> HydrogenFilterwheel::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::error("Unknown type of the Hydrogen filter task: {}", task_name);
        return nullptr;
    }

    std::shared_ptr<Lithium::ConditionalTask> HydrogenFilterwheel::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getCondtionalTask function not implemented");
        return nullptr;
    }

    std::shared_ptr<Lithium::LoopTask> HydrogenFilterwheel::getLoopTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getLoopTask function not implemented");
        return nullptr;
    }
} // namespace Lithium
