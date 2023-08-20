/*
 * inditelescope.cpp
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

#include "lithiumtelescope.hpp"

#include <spdlog/spdlog.h>

namespace Lithium
{
    void INDITelescope::newDevice(LITHIUM::BaseDevice *dp)
    {
        if (dp->getDeviceName() == device_name)
        {
            telescope_device = dp;
        }
    }

    void INDITelescope::newSwitch(ISwitchVectorProperty *svp)
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
                indi_telescope_rate = baud_9600;
            else if (IUFindSwitch(svp, "19200")->s == ISS_ON)
                indi_telescope_rate = baud_19200;
            else if (IUFindSwitch(svp, "38400")->s == ISS_ON)
                indi_telescope_rate = baud_38400;
            else if (IUFindSwitch(svp, "57600")->s == ISS_ON)
                indi_telescope_rate = baud_57600;
            else if (IUFindSwitch(svp, "115200")->s == ISS_ON)
                indi_telescope_rate = baud_115200;
            else if (IUFindSwitch(svp, "230400")->s == ISS_ON)
                indi_telescope_rate = baud_230400;

            spdlog::debug("{} baud rate : {}", _name, indi_telescope_rate);
        }
    }

    void INDITelescope::newMessage(LITHIUM::BaseDevice *dp, int messageID)
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

    void INDITelescope::newNumber(INumberVectorProperty *nvp)
    {
        std::ostringstream os;
        for (int i = 0; i < nvp->nnp; i++)
        {
            if (i)
                os << ',';
            os << nvp->np[i].name << ':' << nvp->np[i].value;
        }
        spdlog::debug("{} Received Number: {} = {} state = {}", _name, nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == telescopeinfo_prop)
        {
            // do nothing
        }
    }

    void INDITelescope::newText(ITextVectorProperty *tvp)
    {
        spdlog::debug("{} Received Text: {} = {}", _name, tvp->name, tvp->tp->text);
    }

    void INDITelescope::newBLOB(IBLOB *bp)
    {
        spdlog::debug("{} Received BLOB {} len = {} size = {}", _name, bp->name, bp->bloblen, bp->size);
    }

    void INDITelescope::newProperty(LITHIUM::Property *property)
    {
        std::string PropName(property->getName());
        LITHIUM_PROPERTY_TYPE Proptype = property->getType();

        spdlog::debug("{} Property: {}", _name, property->getName());

        if (PropName == "DEVICE_PORT" && Proptype == LITHIUM_TEXT)
        {
            spdlog::debug("{} Found device port for {} ", _name, property->getDeviceName());
            telescope_port = property->getText();
        }
        else if (PropName == "CONNECTION" && Proptype == LITHIUM_SWITCH)
        {
            spdlog::debug("{} Found CONNECTION for {} {}", _name, property->getDeviceName(), PropName);
            connection_prop = property->getSwitch();
            ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
            is_connected = (connectswitch->s == ISS_ON);
            if (!is_connected)
            {
                connection_prop->sp->s = ISS_ON;
                sendNewSwitch(connection_prop);
            }
            spdlog::debug("{} Connected {}", _name, is_connected);
        }
        else if (PropName == "DRIVER_INFO" && Proptype == LITHIUM_TEXT)
        {
            device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
            indi_telescope_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
            indi_telescope_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
            indi_telescope_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
            spdlog::debug("{} Name : {} connected exec {}", _name, device_name, indi_telescope_exec);
        }
        else if (PropName == indi_telescope_cmd + "INFO" && Proptype == LITHIUM_NUMBER)
        {
            telescopeinfo_prop = property->getNumber();
            newNumber(telescopeinfo_prop);
        }
        else if (PropName == indi_telescope_cmd + "DEVICE_BAUD_RATE" && Proptype == LITHIUM_SWITCH)
        {
            rate_prop = property->getSwitch();
            if (IUFindSwitch(rate_prop, "9600")->s == ISS_ON)
                indi_telescope_rate = "9600";
            else if (IUFindSwitch(rate_prop, "19200")->s == ISS_ON)
                indi_telescope_rate = "19200";
            else if (IUFindSwitch(rate_prop, "38400")->s == ISS_ON)
                indi_telescope_rate = "38400";
            else if (IUFindSwitch(rate_prop, "57600")->s == ISS_ON)
                indi_telescope_rate = "57600";
            else if (IUFindSwitch(rate_prop, "115200")->s == ISS_ON)
                indi_telescope_rate = "115200";
            else if (IUFindSwitch(rate_prop, "230400")->s == ISS_ON)
                indi_telescope_rate = "230400";
            spdlog::debug("{} baud rate : {}", _name, indi_telescope_rate);
        }
        else if (PropName == indi_telescope_cmd + "DEVICE_PORT" && Proptype == LITHIUM_TEXT)
        {
            indi_telescope_port = IUFindText(property->getText(), "PORT")->text;
            spdlog::debug("{} USB Port : {}", _name, indi_telescope_port);
        }
    }

    void INDITelescope::IndiServerConnected()
    {
        spdlog::debug("{} connection succeeded", _name);
        is_connected = true;
    }

    void INDITelescope::IndiServerDisconnected(int exit_code)
    {
        spdlog::debug("{}: serverDisconnected", _name);
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            spdlog::debug("{} : INDI server disconnected", _name);
    }

    void INDITelescope::removeDevice(LITHIUM::BaseDevice *dp)
    {
        ClearStatus();
        spdlog::info("{} disconnected", _name);
    }

    void INDITelescope::ClearStatus()
    {
        connection_prop = nullptr;
        telescope_port = nullptr;
        telescope_device = nullptr;
        connection_prop = nullptr;
        rate_prop = nullptr;
        telescopeinfo_prop = nullptr;
        telescope_port = nullptr;
        telescope_device = nullptr;
    }

    INDITelescope::INDITelescope(const std::string &name) : Telescope(name)
    {
        spdlog::debug("INDI telescope {} init successfully", name);
    }

    INDITelescope::~INDITelescope()
    {
    }

    bool INDITelescope::connect(std::string name)
    {
        spdlog::debug("Trying to connect to {}", name);
        if (is_connected)
        {
            spdlog::warn("{} is already connected", _name);
            return true;
        }
        if (hostname.empty() || port == 0)
        {
            throw std::runtime_error("Host or port not set!");
        }
        setServer(hostname.c_str(), port);
        watchDevice(name.c_str());
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

    bool INDITelescope::disconnect()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return true;
        }
        disconnectServer();
        is_connected = false;
        return true;
    }

    bool INDITelescope::reconnect()
    {
        return disconnect() and connect(_name);
    }

    bool INDITelescope::scanForAvailableDevices()
    {
        throw std::runtime_error("scanForAvailableDevices function not implemented");
    }

    bool INDITelescope::SlewTo(const std::string &ra, const std::string &dec, const bool j2000)
    {
        throw std::runtime_error("SlewTo function not implemented");
    }

    bool INDITelescope::Abort()
    {
        throw std::runtime_error("Abort function not implemented");
    }

    bool INDITelescope::StartTracking(const std::string &model, const std::string &speed)
    {
        throw std::runtime_error("StartTracking function not implemented");
    }

    bool INDITelescope::StopTracking()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::setTrackingMode(const std::string &mode)
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::setTrackingSpeed(const std::string &speed)
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::Home()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::isAtHome()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::setHomePosition()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::Park()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::Unpark()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::isAtPark()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    bool INDITelescope::setParkPosition()
    {
        if (!is_connected)
        {
            spdlog::warn("{} is not connected", _name);
            return false;
        }
        return true;
    }

    std::shared_ptr<Lithium::SimpleTask> INDITelescope::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
    {
        spdlog::error("Unknown type of the INDI telescope task: {}", task_name);
        return nullptr;
    }

    std::shared_ptr<Lithium::ConditionalTask> INDITelescope::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
    {
        throw std::runtime_error("getCondtionalTask function not implemented");
    }

    std::shared_ptr<Lithium::LoopTask> INDITelescope::getLoopTask(const std::string &task_name, const nlohmann::json &params)
    {
        throw std::runtime_error("getLoopTask function not implemented");
    }

} // namespace Lithium
