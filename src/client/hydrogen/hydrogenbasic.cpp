/*
 * hydrogenbasic.cpp
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

Description: Hydrogen Basic Template

**************************************************/

#include "hydrogenbasic.hpp"

#include "modules/utils/switch.hpp"

#include "config.h"

#include "loguru/loguru.hpp"

HydrogenBasic::HydrogenBasic(const std::string &name)
{
    DLOG_F(INFO, "Hydrogen basic {} init successfully", name);

    m_number_switch = std::make_unique<StringSwitch<INumberVectorProperty *>>();
    m_switch_switch = std::make_unique<StringSwitch<ISwitchVectorProperty *>>();
    m_text_switch = std::make_unique<StringSwitch<ITextVectorProperty *>>();
}

HydrogenBasic::~HydrogenBasic()
{
}

bool HydrogenBasic::connect(const json &params)
{
    std::string name = params["name"];
    std::string hostname = params["host"];
    int port = params["port"];
    DLOG_F(INFO, "Trying to connect to {}", name);
    setServer(hostname.c_str(), port);
    // Receive messages only for our camera.
    watchDevice(name.c_str());
    // Connect to server.
    if (connectServer())
    {
        DLOG_F(INFO, "{}: connectServer done ready", getDeviceName());
        connectDevice(name.c_str());
        return !is_ready.load();
    }
    return false;
}

bool HydrogenBasic::disconnect(const json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool HydrogenBasic::reconnect(const json &params)
{
    return true;
}

bool HydrogenBasic::isConnected()
{
    return true;
}

void HydrogenBasic::newDevice(HYDROGEN::BaseDevice *dp)
{
    if (strcmp(dp->getDeviceName(), getDeviceName().c_str()) == 0)
    {
        telescope_device = dp;
    }
}

void HydrogenBasic::newSwitch(ISwitchVectorProperty *svp)
{
    m_switch_switch->match(svp->name, svp);
}

void HydrogenBasic::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
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

void HydrogenBasic::newNumber(INumberVectorProperty *nvp)
{
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenBasic::newText(ITextVectorProperty *tvp)
{
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenBasic::newBLOB(IBLOB *bp)
{
    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", _name, bp->name, bp->bloblen, bp->size);
}

void HydrogenBasic::newProperty(HYDROGEN::Property *property)
{
    std::string PropName(property->getName());
    HYDROGEN_PROPERTY_TYPE Proptype = property->getType();

    // DLOG_F(INFO,"{} Property: {}", getDeviceName(), property->getName());

    if (Proptype == HYDROGEN_NUMBER)
    {
        newNumber(property->getNumber());
    }
    else if (Proptype == HYDROGEN_SWITCH)
    {
        newSwitch(property->getSwitch());
    }
    else if (Proptype == HYDROGEN_TEXT)
    {
        newText(property->getText());
    }
}

void HydrogenBasic::IndiServerConnected()
{
    DLOG_F(INFO, "{} connection succeeded", _name);
    is_connected.store(true);
}

void HydrogenBasic::IndiServerDisconnected(int exit_code)
{
    DLOG_F(INFO, "{}: serverDisconnected", _name);
    // after disconnection we reset the connection status and the properties pointers
    ClearStatus();
    // in case the connection lost we must reset the client socket
    if (exit_code == -1)
        DLOG_F(INFO, "{} : Hydrogen server disconnected", _name);
}

void HydrogenBasic::removeDevice(HYDROGEN::BaseDevice *dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", _name);
}

void HydrogenBasic::ClearStatus()
{
}
