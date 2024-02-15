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

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenFocuser::HydrogenFocuser(const std::string &name) : Focuser(name)
{
    DLOG_F(INFO, "Hydrogen Focuser {} init successfully", name);

    m_number_switch = std::make_unique<Atom::Utils::StringSwitch<HYDROGEN::PropertyViewNumber *>>();
    m_switch_switch = std::make_unique<Atom::Utils::StringSwitch<HYDROGEN::PropertyViewSwitch *>>();
    m_text_switch = std::make_unique<Atom::Utils::StringSwitch<HYDROGEN::PropertyViewText *>>();

    m_switch_switch->registerCase("CONNECTION", [this](HYDROGEN::PropertyViewSwitch *svp)
                                  {
        m_connection_prop.reset(svp);
        if (auto connectswitch = IUFindSwitch(svp, "CONNECT"); connectswitch->s == ISS_ON)
        {
            SetVariable("connect", true);
            is_connected.store(true);
            DLOG_F(INFO, "{} is connected", GetName());
        }
        else
        {
            if (is_ready.load())
            {
                SetVariable("connect", false);
                is_connected.store(true);
                DLOG_F(INFO, "{} is disconnected", GetName());
            }
        } });

    m_switch_switch->registerCase("DEVICE_BAUD_RATE", [this](HYDROGEN::PropertyViewSwitch *svp)
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

        DLOG_F(INFO, "{} baud rate: {}", GetName(), hydrogen_focuser_rate); });

    m_switch_switch->registerCase("Mode", [this](HYDROGEN::PropertyViewSwitch *svp)
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

    m_switch_switch->registerCase(hydrogen_focuser_cmd + "FOCUS_MOTION", [this](HYDROGEN::PropertyViewSwitch *svp)
                                  { 
                                    m_motion_prop.reset(svp);
                                    m_current_motion.store((IUFindSwitch(svp, "FOCUS_INWARD")->s == ISS_ON) ? 0: 1); });

    m_switch_switch->registerCase(hydrogen_focuser_cmd + "FOCUS_BACKLASH_TOGGLE", [this](HYDROGEN::PropertyViewSwitch *svp)
                                  { 
                                    m_backlash_prop.reset(svp);
                                    has_backlash = (IUFindSwitch(svp, "HYDROGEN_ENABLED")->s == ISS_ON); });

    m_number_switch->registerCase("FOCUS_ABSOLUTE_POSITION", [this](HYDROGEN::PropertyViewNumber *nvp)
                                  {
        m_absolute_position_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "FOCUS_ABSOLUTE_POSITION");
        if (num_value)
        {
            m_current_absolute_position.store(num_value->value);
             DLOG_F(INFO, "{} Current Absolute Position: {}", GetName(), m_current_absolute_position.load());
        } });

    m_number_switch->registerCase("FOCUS_SPEED", [this](HYDROGEN::PropertyViewNumber *nvp)
                                  {
                                      m_speed_prop.reset(nvp);
                                      INumber *num_value = IUFindNumber(nvp, "FOCUS_SPEED");
                                      if (num_value)
                                      {
                                          m_current_speed.store(num_value->value);
                                          DLOG_F(INFO, "{} Current Speed: {}", GetName(), m_current_speed.load());
                                      }
                                  });

    m_number_switch->registerCase("ABS_FOCUS_POSITION", [this](HYDROGEN::PropertyViewNumber *nvp) {

    });

    m_number_switch->registerCase("DELAY", [this](HYDROGEN::PropertyViewNumber *nvp)
                                  {
        m_delay_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "DELAY");
        if(num_value)
        {
            m_delay = num_value->value;
            DLOG_F(INFO, "{} Current Delay: {}", GetName(), m_delay);
        } });

    m_number_switch->registerCase("FOCUS_TEMPERATURE", [this](HYDROGEN::PropertyViewNumber *nvp)
                                  {
        m_temperature_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "FOCUS_TEMPERATURE");
        if(num_value)
        {
            m_current_temperature.store(num_value->value);
            DLOG_F(INFO, "{} Current Temperature: {}", GetName(), m_current_temperature.load());
        } });

    m_number_switch->registerCase("FOCUS_MAX", [this](HYDROGEN::PropertyViewNumber *nvp)
                                  {
        m_max_position_prop.reset(nvp);
        INumber *num_value = IUFindNumber(nvp, "FOCUS_MAX");
        if(num_value)
        {
            m_max_position = num_value->value;
            DLOG_F(INFO, "{} Current Speed: {}", GetName(), m_max_position);
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
        DLOG_F(INFO, "{}: connectServer done ready", GetName());
        connectDevice(name.c_str());
        return !is_ready.load();
    }
    return false;
}

bool HydrogenFocuser::disconnect(const json &params)
{
    DLOG_F(INFO, "%s is disconnected", GetName());
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

void HydrogenFocuser::newDevice(HYDROGEN::BaseDevice dp)
{
    if (dp.getDeviceName() == GetName().c_str())
    {
        focuser_device = dp;
    }
}

void HydrogenFocuser::newSwitch(HYDROGEN::PropertyViewSwitch *svp)
{
    m_switch_switch->match(svp->name, svp);
}

void HydrogenFocuser::newMessage(HYDROGEN::BaseDevice dp, int messageID)
{
    DLOG_F(INFO, "{} Received message: {}", GetName(), dp.messageQueue(messageID));
}

void HydrogenFocuser::serverConnected()
{
    DLOG_F(INFO, "{} Connected to server", GetName());
}

void HydrogenFocuser::serverDisconnected(int exit_code)
{
    DLOG_F(INFO, "{} Disconnected from server", GetName());

    ClearStatus();
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

void HydrogenFocuser::newNumber(HYDROGEN::PropertyViewNumber *nvp)
{
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenFocuser::newText(HYDROGEN::PropertyViewText *tvp)
{
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenFocuser::newBLOB(HYDROGEN::PropertyViewBlob *bp)
{
    DLOG_F(INFO, "{} Received BLOB {}", GetName(), bp->name);
}

void HydrogenFocuser::newProperty(HYDROGEN::Property property)
{
    std::string PropName(property.getName());
    HYDROGEN_PROPERTY_TYPE Proptype = property.getType();

    DLOG_F(INFO,"{} Property: {}", GetName(), property.getName());

    switch (property.getType())
    {
    case HYDROGEN_SWITCH:
    {
        auto svp = property.getSwitch();
        DLOG_F(INFO, "{}: {}", GetName(), svp->name);
        newSwitch(svp);
    }
    break;
    case HYDROGEN_NUMBER:
    {
        auto nvp = property.getNumber();
        DLOG_F(INFO, "{}: {}", GetName(), nvp->name);
        newNumber(nvp);
    }
    break;
    case HYDROGEN_TEXT:
    {
        auto tvp = property.getText();
        DLOG_F(INFO, "{}: {}", GetName(), tvp->name);
        newText(tvp);
    }
    break;
    default:
        break;
    };
}

void HydrogenFocuser::removeDevice(HYDROGEN::BaseDevice dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", GetName());
}

void HydrogenFocuser::ClearStatus()
{
    m_connection_prop = nullptr;
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
}
