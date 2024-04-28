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

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenFilterwheel::HydrogenFilterwheel(const std::string &name)
    : Filterwheel(name) {
    DLOG_F(INFO, "Hydrogen filterwheel {} init successfully", name);

    m_number_switch = std::make_unique<
        atom::utils::StringSwitch<HYDROGEN::PropertyViewNumber *>>();
    m_switch_switch = std::make_unique<
        atom::utils::StringSwitch<HYDROGEN::PropertyViewSwitch *>>();
    m_text_switch = std::make_unique<
        atom::utils::StringSwitch<HYDROGEN::PropertyViewText *>>();

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

            DLOG_F(INFO, "{} baud rate : {}", GetName(), hydrogen_filter_rate);
        });

    m_text_switch->registerCase(
        "DEVICE_PORT", [this](HYDROGEN::PropertyViewText *tvp) {
            filter_prop.reset(tvp);
            hydrogen_filter_port = tvp->tp->text;
            SetVariable("port", hydrogen_filter_port);
            DLOG_F(INFO, "Current device port of {} is {}", GetName(),
                   filter_prop->tp->text);
        });

    m_text_switch->registerCase(
        "DRIVER_INFO", [this](HYDROGEN::PropertyViewText *tvp) {
            hydrogen_filter_exec = IUFindText(tvp, "DRIVER_EXEC")->text;
            hydrogen_filter_version = IUFindText(tvp, "DRIVER_VERSION")->text;
            hydrogen_filter_interface =
                IUFindText(tvp, "DRIVER_INTERFACE")->text;
            DLOG_F(INFO, "Filterwheel Name : {} connected exec {}", GetName(),
                   GetName(), hydrogen_filter_exec);
        });
}

HydrogenFilterwheel::~HydrogenFilterwheel() {}

bool HydrogenFilterwheel::connect(const json &params) {
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

bool HydrogenFilterwheel::disconnect(const json &params) {
    DLOG_F(INFO, "%s is disconnected", GetName());
    return true;
}

bool HydrogenFilterwheel::reconnect(const json &params) { return true; }

bool HydrogenFilterwheel::isConnected() { return true; }

bool HydrogenFilterwheel::moveTo(const json &params) { return true; }

bool HydrogenFilterwheel::getCurrentPosition(const json &params) {
    return true;
}

void HydrogenFilterwheel::newDevice(HYDROGEN::BaseDevice dp) {
    if (strcmp(dp.getDeviceName(), GetName().c_str()) == 0) {
        filter_device = dp;
    }
}

void HydrogenFilterwheel::removeDevice(HYDROGEN::BaseDevice dp) {
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", GetName());
}

void HydrogenFilterwheel::newSwitch(HYDROGEN::PropertyViewSwitch *svp) {
    m_switch_switch->match(svp->name, svp);
}

void HydrogenFilterwheel::newMessage(HYDROGEN::BaseDevice dp, int messageID) {
    DLOG_F(INFO, "{} Received message: {}", GetName(),
           dp.messageQueue(messageID));
}

void HydrogenFilterwheel::serverConnected() {
    DLOG_F(INFO, "{} Connected to server", GetName());
}

void HydrogenFilterwheel::serverDisconnected(int exit_code) {
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

void HydrogenFilterwheel::newNumber(HYDROGEN::PropertyViewNumber *nvp) {
    m_number_switch->match(nvp->name, nvp);
}

void HydrogenFilterwheel::newText(HYDROGEN::PropertyViewText *tvp) {
    m_text_switch->match(tvp->name, tvp);
}

void HydrogenFilterwheel::newBLOB(HYDROGEN::PropertyViewBlob *bp) {
    DLOG_F(INFO, "{} Received BLOB {}", GetName(), bp->name);
}

void HydrogenFilterwheel::newProperty(HYDROGEN::Property property) {
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
    }
}

void HydrogenFilterwheel::ClearStatus() {
    m_connection_prop = nullptr;
    filterinfo_prop = nullptr;
    filter_port = nullptr;
    rate_prop = nullptr;
    filter_prop = nullptr;
}
