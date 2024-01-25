/*
 * hydrogenfilterwheel.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-10

Description: Hydrogen Filterwheel

**************************************************/

#include "hydrogenfilterwheel.hpp"

#include "atom/utils/switch.hpp"

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenFilterwheel::HydrogenFilterwheel(const std::string &name) : Filterwheel(name)
{
    DLOG_F(INFO, "Hydrogen filterwheel {} init successfully", name);

    m_number_switch = std::make_unique<StringSwitch<INumberVectorProperty *>>();
    m_switch_switch = std::make_unique<StringSwitch<ISwitchVectorProperty *>>();
    m_text_switch = std::make_unique<StringSwitch<ITextVectorProperty *>>();

    m_switch_switch->registerCase("CONNECTION", [this](ISwitchVectorProperty *svp)
                                  {
        m_connection_prop.reset(svp);
        if (auto connectswitch = IUFindSwitch(svp, "CONNECT"); connectswitch->s == ISS_ON)
        {
            setProperty("connect", true);
            is_connected.store(true);
            DLOG_F(INFO, "{} is connected", getDeviceName());
        }
        else
        {
            if (is_ready.load())
            {
                setProperty("connect", false);
                is_connected.store(true);
                DLOG_F(INFO, "{} is disconnected", getDeviceName());
            }
        } });

    m_switch_switch->registerCase("DEVICE_BAUD_RATE", [this](ISwitchVectorProperty *svp)
                                  {
        std::string const baud_9600{"9600"};
        std::string const baud_19200{"19200"};
        std::string const baud_38400{"38400"};
        std::string const baud_57600{"57600"};
        std::string const baud_115200{"115200"};
        std::string const baud_230400{"230400"};

        if (IUFindSwitch(svp, "9600")->s == ISS_ON)
            hydrogen_filter_rate = baud_9600;
        else if (IUFindSwitch(svp, "19200")->s == ISS_ON)
            hydrogen_filter_rate = baud_19200;
        else if (IUFindSwitch(svp, "38400")->s == ISS_ON)
            hydrogen_filter_rate = baud_38400;
        else if (IUFindSwitch(svp, "57600")->s == ISS_ON)
            hydrogen_filter_rate = baud_57600;
        else if (IUFindSwitch(svp, "115200")->s == ISS_ON)
            hydrogen_filter_rate = baud_115200;
        else if (IUFindSwitch(svp, "230400")->s == ISS_ON)
            hydrogen_filter_rate = baud_230400;

        DLOG_F(INFO, "{} baud rate : {}", getDeviceName(), hydrogen_filter_rate); });

    m_text_switch->registerCase("DEVICE_PORT", [this](ITextVectorProperty *tvp)
                                {
                                    filter_prop.reset(tvp);
                                    hydrogen_filter_port = tvp->tp->text;
                                    setProperty("port", hydrogen_filter_port);
                                    DLOG_F(INFO, "Current device port of {} is {}", getDeviceName(), filter_prop->tp->text); });

    m_text_switch->registerCase("DRIVER_INFO", [this](ITextVectorProperty *tvp)
                                {
        hydrogen_filter_exec = IUFindText(tvp, "DRIVER_EXEC")->text;
        hydrogen_filter_version = IUFindText(tvp, "DRIVER_VERSION")->text;
        hydrogen_filter_interface = IUFindText(tvp, "DRIVER_INTERFACE")->text;
        DLOG_F(INFO, "Filterwheel Name : {} connected exec {}", getDeviceName(), getDeviceName(), hydrogen_filter_exec); });
}

HydrogenFilterwheel::~HydrogenFilterwheel()
{
}

bool HydrogenFilterwheel::connect(const json &params)
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

bool HydrogenFilterwheel::disconnect(const json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool HydrogenFilterwheel::reconnect(const json &params)
{
    return true;
}

bool HydrogenFilterwheel::isConnected()
{
    return true;
}

bool HydrogenFilterwheel::moveTo(const json &params)
{
    return true;
}

bool HydrogenFilterwheel::getCurrentPosition(const json &params)
{
    return true;
}

void HydrogenFilterwheel::newDevice(HYDROGEN::BaseDevice *dp)
{
    if (strcmp(dp->getDeviceName(), getDeviceName().c_str()) == 0)
    {
        filter_device = dp;
    }
}

void HydrogenFilterwheel::newSwitch(ISwitchVectorProperty *svp)
{
    m_switch_switch->match(svp->name, svp);
}

void HydrogenFilterwheel::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
{
    DLOG_F(INFO, "{} Received message: {}", getDeviceName(), dp->messageQueue(messageID));
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
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenFilterwheel::newText(ITextVectorProperty *tvp)
{
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenFilterwheel::newBLOB(IBLOB *bp)
{
    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", getDeviceName(), bp->name, bp->bloblen, bp->size);
}

void HydrogenFilterwheel::newProperty(HYDROGEN::Property *property)
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

void HydrogenFilterwheel::IndiServerConnected()
{
    DLOG_F(INFO, "{} connection succeeded", getDeviceName());
    is_connected = true;
}

void HydrogenFilterwheel::IndiServerDisconnected(int exit_code)
{
    DLOG_F(INFO, "{}: serverDisconnected", getDeviceName());
    // after disconnection we reset the connection status and the properties pointers
    ClearStatus();
    // in case the connection lost we must reset the client socket
    if (exit_code == -1)
        DLOG_F(INFO, "{} : Hydrogen server disconnected", getDeviceName());
}

void HydrogenFilterwheel::removeDevice(HYDROGEN::BaseDevice *dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", getDeviceName());
}

void HydrogenFilterwheel::ClearStatus()
{
    m_connection_prop = nullptr;
    filterinfo_prop = nullptr;
    filter_port = nullptr;
    rate_prop = nullptr;
    filter_prop = nullptr;
}
