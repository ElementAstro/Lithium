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

Description: INDI Filterwheel

**************************************************/

#include "indifilterwheel.hpp"

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    void INDIFilterwheel::newDevice(INDI::BaseDevice *dp)
    {
        if (dp->getDeviceName() == device_name)
        {
            filter_device = dp;
        }
    }

    void INDIFilterwheel::newSwitch(ISwitchVectorProperty *svp)
    {
        std::string const connection_str{"CONNECTION"};
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

            spdlog::debug("{} baud rate : {}", _name, indi_filter_rate);
        }
    }

    void INDIFilterwheel::newMessage(INDI::BaseDevice *dp, int messageID)
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

    void INDIFilterwheel::newNumber(INumberVectorProperty *nvp)
    {
        std::ostringstream os;
        for (int i = 0; i < nvp->nnp; i++)
        {
            if (i)
                os << ',';
            os << nvp->np[i].name << ':' << nvp->np[i].value;
        }
        spdlog::debug("{} Received Number: {} = {} state = {}", _name, nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == filterinfo_prop)
        {
            // do nothing
        }
    }

    void INDIFilterwheel::newText(ITextVectorProperty *tvp)
    {
        spdlog::debug("{} Received Text: {} = {}", _name, tvp->name, tvp->tp->text);
    }

    void INDIFilterwheel::newBLOB(IBLOB *bp)
    {
        spdlog::debug("{} Received BLOB {} len = {} size = {}",_name, bp->name, bp->bloblen, bp->size);
    }

    void INDIFilterwheel::newProperty(INDI::Property *property)
    {
        std::string PropName(property->getName());
        INDI_PROPERTY_TYPE Proptype = property->getType();

        spdlog::debug("{} Property: {}",_name, property->getName());

        if (PropName == "DEVICE_PORT" && Proptype == INDI_TEXT)
        {
            spdlog::debug("{} Found device port for {} ",_name, property->getDeviceName());
            filter_port = property->getText();
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
            indi_filter_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
            indi_filter_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
            indi_filter_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
            spdlog::debug("{} Name : {} connected exec {}",_name, device_name, indi_filter_exec);
        }
        else if (PropName == indi_filter_cmd + "INFO" && Proptype == INDI_NUMBER)
        {
            filterinfo_prop = property->getNumber();
            newNumber(filterinfo_prop);
        }
        else if (PropName == indi_filter_cmd + "DEVICE_BAUD_RATE" && Proptype == INDI_SWITCH)
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
            spdlog::debug("{} baud rate : {}",_name, indi_filter_rate);
        }
        else if (PropName == indi_filter_cmd + "DEVICE_PORT" && Proptype == INDI_TEXT)
        {
            indi_filter_port = IUFindText(property->getText(), "PORT")->text;
            spdlog::debug("{} USB Port : {}",_name, indi_filter_port);
        }
    }

    void INDIFilterwheel::IndiServerConnected()
    {
        spdlog::debug("{} connection succeeded",_name);
        is_connected = true;
    }

    void INDIFilterwheel::IndiServerDisconnected(int exit_code)
    {
        spdlog::debug("{}: serverDisconnected",_name);
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            spdlog::debug("{} : INDI server disconnected",_name);
    }

    void INDIFilterwheel::removeDevice(INDI::BaseDevice *dp)
    {
        ClearStatus();
        spdlog::info("{} disconnected",_name);
    }

    void INDIFilterwheel::ClearStatus()
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

    INDIFilterwheel::INDIFilterwheel(const std::string &name) : Filterwheel(name)
    {
        spdlog::debug("INDI filterwheel {} init successfully",name);
    }

    INDIFilterwheel::~INDIFilterwheel()
    {
    }

    bool INDIFilterwheel::connect(std::string name)
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

    bool INDIFilterwheel::disconnect()
    {
        return true;
    }

    bool INDIFilterwheel::reconnect()
    {
        disconnect();
        return connect(_name);
    }

    bool INDIFilterwheel::scanForAvailableDevices()
    {
        spdlog::warn("scanForAvailableDevices function not implemented");
        return false;
    }

    std::shared_ptr<OpenAPT::SimpleTask> INDIFilterwheel::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::error("Unknown type of the INDI filter task: {}", task_name);
        return nullptr;
    }

    std::shared_ptr<OpenAPT::ConditionalTask> INDIFilterwheel::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getCondtionalTask function not implemented");
        return nullptr;
    }

    std::shared_ptr<OpenAPT::LoopTask> INDIFilterwheel::getLoopTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::warn("getLoopTask function not implemented");
        return nullptr;
    }
} // namespace OpenAPT
