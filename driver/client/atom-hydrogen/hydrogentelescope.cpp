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

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenTelescope::HydrogenTelescope(const std::string &name)
    : Telescope(name) {
    DLOG_F(INFO, "Hydrogen telescope {} init successfully", name);

    m_number_switch = std::make_unique<
        Atom::Utils::StringSwitch<HYDROGEN::PropertyViewNumber *>>();
    m_switch_switch = std::make_unique<
        Atom::Utils::StringSwitch<HYDROGEN::PropertyViewSwitch *>>();
    m_text_switch = std::make_unique<
        Atom::Utils::StringSwitch<HYDROGEN::PropertyViewText *>>();

    m_switch_switch->registerCase(
        "CONNECTION", [this](HYDROGEN::PropertyViewSwitch *svp) {
            m_connection_prop.reset(svp);
            if (auto connectswitch = IUFindSwitch(svp, "CONNECT");
                connectswitch->s == ISS_ON) {
                SetVariable("connect", true);
                is_connected.store(true);
                DLOG_F(INFO, "{} is connected", GetName());
            } else {
                if (is_ready.load()) {
                    SetVariable("connect", false);
                    is_connected.store(true);
                    DLOG_F(INFO, "{} is disconnected", GetName());
                }
            }
        });

    m_switch_switch->registerCase(
        "DEVICE_BAUD_RATE", [this](HYDROGEN::PropertyViewSwitch *svp) {
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

            DLOG_F(INFO, "{} baud rate : {}", GetName(),
                   hydrogen_telescope_rate);
        });

    m_text_switch->registerCase(
        "DEVICE_PORT", [this](HYDROGEN::PropertyViewText *tvp) {
            telescope_prop.reset(tvp);
            hydrogen_telescope_port = tvp->tp->text;
            SetVariable("port", hydrogen_telescope_port);
            DLOG_F(INFO, "Current device port of {} is {}", GetName(),
                   telescope_prop->tp->text);
        });

    m_text_switch->registerCase(
        "DRIVER_INFO", [this](HYDROGEN::PropertyViewText *tvp) {
            hydrogen_telescope_exec = IUFindText(tvp, "DRIVER_EXEC")->text;
            hydrogen_telescope_version =
                IUFindText(tvp, "DRIVER_VERSION")->text;
            hydrogen_telescope_interface =
                IUFindText(tvp, "DRIVER_INTERFACE")->text;
            DLOG_F(INFO, "Telescope Name : {} connected exec {}", GetName(),
                   GetName(), hydrogen_telescope_exec);
        });
}

HydrogenTelescope::~HydrogenTelescope() {}

bool HydrogenTelescope::connect(const json &params) {
    std::string name = params["name"];
    std::string hostname = params["host"];
    int port = params["port"];
    DLOG_F(INFO, "Trying to connect to {}", name);
    setServer(hostname.c_str(), port);
    // Receive messages only for our camera.
    watchDevice(name.c_str());
    // Connect to server.
    if (connectServer()) {
        DLOG_F(INFO, "{}: connectServer done ready", GetName());
        connectDevice(name.c_str());
        return !is_ready.load();
    }
    return false;
}

bool HydrogenTelescope::disconnect(const json &params) {
    DLOG_F(INFO, "%s is disconnected", GetName());
    return true;
}

bool HydrogenTelescope::reconnect(const json &params) { return true; }

bool HydrogenTelescope::isConnected() { return true; }

bool HydrogenTelescope::SlewTo(const json &params) { return true; }

bool HydrogenTelescope::Abort(const json &params) { return true; }

bool HydrogenTelescope::isSlewing(const json &params) { return true; }

std::string HydrogenTelescope::getCurrentRA(const json &params) { return ""; }

std::string HydrogenTelescope::getCurrentDec(const json &params) { return ""; }

bool HydrogenTelescope::StartTracking(const json &params) { return true; }

bool HydrogenTelescope::StopTracking(const json &params) { return true; }

bool HydrogenTelescope::setTrackingMode(const json &params) { return true; }

bool HydrogenTelescope::setTrackingSpeed(const json &params) { return true; }

std::string HydrogenTelescope::getTrackingMode(const json &params) {
    return "";
}

std::string HydrogenTelescope::getTrackingSpeed(const json &params) {
    return "";
}

bool HydrogenTelescope::Home(const json &params) { return true; }

bool HydrogenTelescope::isAtHome(const json &params) { return true; }

bool HydrogenTelescope::setHomePosition(const json &params) { return true; }

bool HydrogenTelescope::isHomeAvailable(const json &params) { return true; }

bool HydrogenTelescope::Park(const json &params) { return true; }

bool HydrogenTelescope::Unpark(const json &params) { return true; }

bool HydrogenTelescope::isAtPark(const json &params) { return true; }

bool HydrogenTelescope::setParkPosition(const json &params) { return true; }

bool HydrogenTelescope::isParkAvailable(const json &params) { return true; }

void HydrogenTelescope::newDevice(HYDROGEN::BaseDevice dp) {
    if (strcmp(dp.getDeviceName(), GetName().c_str()) == 0) {
        telescope_device = dp;
    }
}

void HydrogenTelescope::newSwitch(HYDROGEN::PropertyViewSwitch *svp) {
    m_switch_switch->match(svp->name, svp);
}

void HydrogenTelescope::newMessage(HYDROGEN::BaseDevice dp, int messageID) {
    DLOG_F(INFO, "{} Received message: {}", GetName(),
           dp.messageQueue(messageID));
}

void HydrogenTelescope::serverConnected() {
    DLOG_F(INFO, "{} Connected to server", GetName());
}

void HydrogenTelescope::serverDisconnected(int exit_code) {
    DLOG_F(INFO, "{} Disconnected from server", GetName());

    ClearStatus();
}

inline static const char *StateStr(IPState st) {
    switch (st) {
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

void HydrogenTelescope::newNumber(HYDROGEN::PropertyViewNumber *nvp) {
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenTelescope::newText(HYDROGEN::PropertyViewText *tvp) {
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenTelescope::newBLOB(HYDROGEN::PropertyViewBlob *bp) {
    DLOG_F(INFO, "{} Received BLOB {}", GetName(), bp->name);
}

void HydrogenTelescope::newProperty(HYDROGEN::Property property) {
    std::string PropName(property.getName());
    HYDROGEN_PROPERTY_TYPE Proptype = property.getType();

    DLOG_F(INFO, "{} Property: {}", GetName(), property.getName());

    switch (property.getType()) {
        case HYDROGEN_SWITCH: {
            auto svp = property.getSwitch();
            DLOG_F(INFO, "{}: {}", GetName(), svp->name);
            newSwitch(svp);
        } break;
        case HYDROGEN_NUMBER: {
            auto nvp = property.getNumber();
            DLOG_F(INFO, "{}: {}", GetName(), nvp->name);
            newNumber(nvp);
        } break;
        case HYDROGEN_TEXT: {
            auto tvp = property.getText();
            DLOG_F(INFO, "{}: {}", GetName(), tvp->name);
            newText(tvp);
        } break;
        default:
            break;
    };
}

void HydrogenTelescope::removeDevice(HYDROGEN::BaseDevice dp) {
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", GetName());
}

void HydrogenTelescope::ClearStatus() {
    m_connection_prop = nullptr;
    telescope_port = nullptr;
    m_connection_prop = nullptr;
    rate_prop = nullptr;
    telescopeinfo_prop = nullptr;
    telescope_port = nullptr;
}
