/*
 * hydrogenfocuser.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-10

Description: Hydrogen Focuser

**************************************************/

#include "hydrogenfocuser.hpp"

#include "atom/utils/switch.hpp"

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenFocuser::HydrogenFocuser(const std::string &name) : Focuser(name)
{
    DLOG_F(INFO, "Hydrogen Focuser {} init successfully", name);

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
            hydrogen_focuser_rate = baud_9600;
        else if (IUFindSwitch(svp, "19200")->s == ISS_ON)
            hydrogen_focuser_rate = baud_19200;
        else if (IUFindSwitch(svp, "38400")->s == ISS_ON)
            hydrogen_focuser_rate = baud_38400;
        else if (IUFindSwitch(svp, "57600")->s == ISS_ON)
            hydrogen_focuser_rate = baud_57600;
        else if (IUFindSwitch(svp, "115200")->s == ISS_ON)
            hydrogen_focuser_rate = baud_115200;
        else if (IUFindSwitch(svp, "230400")->s == ISS_ON)
            hydrogen_focuser_rate = baud_230400;

        DLOG_F(INFO, "{} baud rate: {}", getDeviceName(), hydrogen_focuser_rate); });

    m_switch_switch->registerCase("Mode", [this](ISwitchVectorProperty *svp)
                                  {
        m_mode_prop.reset(svp);
        ISwitch *modeswitch = IUFindSwitch(svp, "All");
        if (modeswitch->s == ISS_ON)
        {
            can_absolute_move = true;
            current_mode.store(0);
        }
        else
        {
            modeswitch = IUFindSwitch(svp, "Absolute");
            if (modeswitch->s == ISS_ON)
            {
                can_absolute_move = true;
                current_mode.store(1);
            }
            else
            {
                can_absolute_move = false;
                current_mode.store(2);
            }
        } });

    m_switch_switch->registerCase(hydrogen_focuser_cmd + "FOCUS_MOTION", [this](ISwitchVectorProperty *svp)
                                  { 
                                    m_motion_prop.reset(svp);
                                    m_current_motion.store((IUFindSwitch(svp, "FOCUS_INWARD")->s == ISS_ON) ? 0: 1); });

    m_switch_switch->registerCase(hydrogen_focuser_cmd + "FOCUS_BACKLASH_TOGGLE", [this](ISwitchVectorProperty *svp)
                                  { 
                                    m_backlash_prop.reset(svp);
                                    has_backlash = (IUFindSwitch(svp, "HYDROGEN_ENABLED")->s == ISS_ON); });

    m_number_switch->registerCase("FOCUS_ABSOLUTE_POSITION", [this](INumberVectorProperty *nvp)
                                  {
        m_absolute_position_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "FOCUS_ABSOLUTE_POSITION");
        if (num_value)
        {
            m_current_absolute_position.store(num_value->value);
             DLOG_F(INFO, "{} Current Absolute Position: {}", getDeviceName(), m_current_absolute_position.load());
        } });

    m_number_switch->registerCase("FOCUS_SPEED", [this](INumberVectorProperty *nvp)
                                  {
                                      m_speed_prop.reset(nvp);
                                      INumber *num_value = IUFindNumber(nvp, "FOCUS_SPEED");
                                      if (num_value)
                                      {
                                          m_current_speed.store(num_value->value);
                                          DLOG_F(INFO, "{} Current Speed: {}", getDeviceName(), m_current_speed.load());
                                      }
                                  });

    m_number_switch->registerCase("ABS_FOCUS_POSITION", [this](INumberVectorProperty *nvp) {

    });

    m_number_switch->registerCase("DELAY", [this](INumberVectorProperty *nvp)
                                  {
        m_delay_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "DELAY");
        if(num_value)
        {
            m_delay = num_value->value;
            DLOG_F(INFO, "{} Current Delay: {}", getDeviceName(), m_delay);
        } });

    m_number_switch->registerCase("FOCUS_TEMPERATURE", [this](INumberVectorProperty *nvp)
                                  {
        m_temperature_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "FOCUS_TEMPERATURE");
        if(num_value)
        {
            m_current_temperature.store(num_value->value);
            DLOG_F(INFO, "{} Current Temperature: {}", getDeviceName(), m_current_temperature.load());
        } });

    m_number_switch->registerCase("FOCUS_MAX", [this](INumberVectorProperty *nvp)
                                  {
        m_max_position_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "FOCUS_MAX");
        if(num_value)
        {
            m_max_position = num_value->value;
            DLOG_F(INFO, "{} Current Speed: {}", getDeviceName(), m_max_position);
        } });
}

HydrogenFocuser::~HydrogenFocuser()
{
}

bool HydrogenFocuser::connect(const json &params)
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

bool HydrogenFocuser::disconnect(const json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool HydrogenFocuser::reconnect(const json &params)
{
    return true;
}

bool HydrogenFocuser::isConnected()
{
    return true;
}

bool HydrogenFocuser::moveTo(const json &params)
{
    return true;
}

bool HydrogenFocuser::moveToAbsolute(const json &params)
{
    return true;
}

bool HydrogenFocuser::moveStep(const json &params)
{
    return true;
}

bool HydrogenFocuser::moveStepAbsolute(const json &params)
{
    return true;
}

bool HydrogenFocuser::AbortMove(const json &params)
{
    return true;
}

int HydrogenFocuser::getMaxPosition(const json &params)
{
    return 0;
}

bool HydrogenFocuser::setMaxPosition(const json &params)
{
    return true;
}

bool HydrogenFocuser::isGetTemperatureAvailable(const json &params)
{
    return true;
}

double HydrogenFocuser::getTemperature(const json &params)
{
    return 0.0;
}

bool HydrogenFocuser::isAbsoluteMoveAvailable(const json &params)
{
    return true;
}

bool HydrogenFocuser::isManualMoveAvailable(const json &params)
{
    return true;
}

int HydrogenFocuser::getCurrentPosition(const json &params)
{
    return 0;
}

bool HydrogenFocuser::haveBacklash(const json &params)
{
    return true;
}

bool HydrogenFocuser::setBacklash(const json &params)
{
    return true;
}

void HydrogenFocuser::newDevice(HYDROGEN::BaseDevice *dp)
{
    if (dp->getDeviceName() == getDeviceName())
    {
        focuser_device = dp;
    }
}

void HydrogenFocuser::newSwitch(ISwitchVectorProperty *svp)
{
    m_switch_switch->match(svp->name, svp);
}

void HydrogenFocuser::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
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

void HydrogenFocuser::newNumber(INumberVectorProperty *nvp)
{
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenFocuser::newText(ITextVectorProperty *tvp)
{
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenFocuser::newBLOB(IBLOB *bp)
{
    // we go here every time a new blob is available
    // this is normally the image from the Focuser
    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", getDeviceName(), bp->name, bp->bloblen, bp->size);
}

void HydrogenFocuser::newProperty(HYDROGEN::Property *property)
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

void HydrogenFocuser::IndiServerConnected()
{
    DLOG_F(INFO, "{} connection succeeded", getDeviceName());
    is_connected.store(true);
}

void HydrogenFocuser::IndiServerDisconnected(int exit_code)
{
    DLOG_F(INFO, "{}: serverDisconnected", getDeviceName());
    // after disconnection we reset the connection status and the properties pointers
    ClearStatus();
    // in case the connection lost we must reset the client socket
    if (exit_code == -1)
        DLOG_F(INFO, "{}: Hydrogen server disconnected", getDeviceName());
}

void HydrogenFocuser::removeDevice(HYDROGEN::BaseDevice *dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", getDeviceName());
}

void HydrogenFocuser::ClearStatus()
{
    m_connection_prop = nullptr;
    focuser_port = nullptr;
    focuser_device = nullptr;
    m_connection_prop = nullptr;
    m_mode_prop = nullptr;              // Focuser mode , absolute or relative
    m_motion_prop = nullptr;            // Focuser motion , inward or outward
    m_speed_prop = nullptr;             // Focuser speed , default is 1
    m_absolute_position_prop = nullptr; // Focuser absolute position
    m_relative_position_prop = nullptr; // Focuser relative position
    m_max_position_prop = nullptr;      // Focuser max position
    m_temperature_prop = nullptr;       // Focuser temperature
    m_rate_prop = nullptr;
    m_delay_prop = nullptr;
    m_backlash_prop = nullptr;
    m_hydrogen_max_position = nullptr;
    m_hydrogen_focuser_temperature = nullptr;
    m_focuserinfo_prop = nullptr;
    focuser_port = nullptr;
    focuser_device = nullptr;
}
