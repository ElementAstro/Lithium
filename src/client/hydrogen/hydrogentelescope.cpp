/*
 * hydrogentelescope.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-10

Description: Hydrogen Telescope

**************************************************/

#include "hydrogentelescope.hpp"

#include "atom/utils/switch.hpp"

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenTelescope::HydrogenTelescope(const std::string &name) : Telescope(name)
{
    DLOG_F(INFO, "Hydrogen telescope {} init successfully", name);

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
            hydrogen_telescope_rate = baud_9600;
        else if (IUFindSwitch(svp, "19200")->s == ISS_ON)
            hydrogen_telescope_rate = baud_19200;
        else if (IUFindSwitch(svp, "38400")->s == ISS_ON)
            hydrogen_telescope_rate = baud_38400;
        else if (IUFindSwitch(svp, "57600")->s == ISS_ON)
            hydrogen_telescope_rate = baud_57600;
        else if (IUFindSwitch(svp, "115200")->s == ISS_ON)
            hydrogen_telescope_rate = baud_115200;
        else if (IUFindSwitch(svp, "230400")->s == ISS_ON)
            hydrogen_telescope_rate = baud_230400;

        DLOG_F(INFO, "{} baud rate : {}", getDeviceName(), hydrogen_telescope_rate); });

    m_text_switch->registerCase("DEVICE_PORT", [this](ITextVectorProperty *tvp)
                                {
                                    telescope_prop.reset(tvp);
                                    hydrogen_telescope_port = tvp->tp->text;
                                    setProperty("port", hydrogen_telescope_port);
                                    DLOG_F(INFO, "Current device port of {} is {}", getDeviceName(), telescope_prop->tp->text); });

    m_text_switch->registerCase("DRIVER_INFO", [this](ITextVectorProperty *tvp)
                                {
        hydrogen_telescope_exec = IUFindText(tvp, "DRIVER_EXEC")->text;
        hydrogen_telescope_version = IUFindText(tvp, "DRIVER_VERSION")->text;
        hydrogen_telescope_interface = IUFindText(tvp, "DRIVER_INTERFACE")->text;
        DLOG_F(INFO, "Telescope Name : {} connected exec {}", getDeviceName(), getDeviceName(), hydrogen_telescope_exec); });
}

HydrogenTelescope::~HydrogenTelescope()
{
}

bool HydrogenTelescope::connect(const json &params)
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

bool HydrogenTelescope::disconnect(const json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool HydrogenTelescope::reconnect(const json &params)
{
    return true;
}

bool HydrogenTelescope::isConnected()
{
    return true;
}

bool HydrogenTelescope::SlewTo(const json &params)
{
    return true;
}

bool HydrogenTelescope::Abort(const json &params)
{
    return true;
}

bool HydrogenTelescope::isSlewing(const json &params)
{
    return true;
}

std::string HydrogenTelescope::getCurrentRA(const json &params)
{
    return "";
}

std::string HydrogenTelescope::getCurrentDec(const json &params)
{
    return "";
}

bool HydrogenTelescope::StartTracking(const json &params)
{
    return true;
}

bool HydrogenTelescope::StopTracking(const json &params)
{
    return true;
}

bool HydrogenTelescope::setTrackingMode(const json &params)
{
    return true;
}

bool HydrogenTelescope::setTrackingSpeed(const json &params)
{
    return true;
}

std::string HydrogenTelescope::getTrackingMode(const json &params)
{
    return "";
}

std::string HydrogenTelescope::getTrackingSpeed(const json &params)
{
    return "";
}

bool HydrogenTelescope::Home(const json &params)
{
    return true;
}

bool HydrogenTelescope::isAtHome(const json &params)
{
    return true;
}

bool HydrogenTelescope::setHomePosition(const json &params)
{
    return true;
}

bool HydrogenTelescope::isHomeAvailable(const json &params)
{
    return true;
}

bool HydrogenTelescope::Park(const json &params)
{
    return true;
}

bool HydrogenTelescope::Unpark(const json &params)
{
    return true;
}

bool HydrogenTelescope::isAtPark(const json &params)
{
    return true;
}

bool HydrogenTelescope::setParkPosition(const json &params)
{
    return true;
}

bool HydrogenTelescope::isParkAvailable(const json &params)
{
    return true;
}

void HydrogenTelescope::newDevice(HYDROGEN::BaseDevice *dp)
{
    if (strcmp(dp->getDeviceName(), getDeviceName().c_str()) == 0)
    {
        telescope_device = dp;
    }
}

void HydrogenTelescope::newSwitch(ISwitchVectorProperty *svp)
{
    m_switch_switch->match(svp->name, svp);
}

void HydrogenTelescope::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
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

void HydrogenTelescope::newNumber(INumberVectorProperty *nvp)
{
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenTelescope::newText(ITextVectorProperty *tvp)
{
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenTelescope::newBLOB(IBLOB *bp)
{
    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", getDeviceName(), bp->name, bp->bloblen, bp->size);
}

void HydrogenTelescope::newProperty(HYDROGEN::Property *property)
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

void HydrogenTelescope::IndiServerConnected()
{
    DLOG_F(INFO, "{} connection succeeded", getDeviceName());
    is_connected.store(true);
}

void HydrogenTelescope::IndiServerDisconnected(int exit_code)
{
    DLOG_F(INFO, "{}: serverDisconnected", getDeviceName());
    // after disconnection we reset the connection status and the properties pointers
    ClearStatus();
    // in case the connection lost we must reset the client socket
    if (exit_code == -1)
        DLOG_F(INFO, "{} : Hydrogen server disconnected", getDeviceName());
}

void HydrogenTelescope::removeDevice(HYDROGEN::BaseDevice *dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", getDeviceName());
}

void HydrogenTelescope::ClearStatus()
{
    m_connection_prop = nullptr;
    telescope_port = nullptr;
    telescope_device = nullptr;
    m_connection_prop = nullptr;
    rate_prop = nullptr;
    telescopeinfo_prop = nullptr;
    telescope_port = nullptr;
    telescope_device = nullptr;
}
