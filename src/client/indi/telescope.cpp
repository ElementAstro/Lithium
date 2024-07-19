#include "telescope.hpp"

#include <optional>
#include <tuple>

#include "atom/log/loguru.hpp"

INDITelescope::INDITelescope(std::string name) : name_(name) {}

auto INDITelescope::connect(const std::string &deviceName) -> bool {
    if (isConnected_.load()) {
        LOG_F(ERROR, "{} is already connected.", deviceName_);
        return false;
    }

    deviceName_ = deviceName;
    LOG_F(INFO, "Connecting to {}...", deviceName_);
    // Max: 需要获取初始的参数，然后再注册对应的回调函数
    watchDevice(deviceName_.c_str(), [this](INDI::BaseDevice device) {
        device_ = device;  // save device

        // wait for the availability of the "CONNECTION" property
        device.watchProperty(
            "CONNECTION",
            [this](INDI::Property) {
                LOG_F(INFO, "Connecting to {}...", deviceName_);
                connectDevice(name_.c_str());
            },
            INDI::BaseDevice::WATCH_NEW);

        device.watchProperty(
            "CONNECTION",
            [this](const INDI::PropertySwitch &property) {
                isConnected_ = property[0].getState() == ISS_ON;
                if (isConnected_.load()) {
                    LOG_F(INFO, "{} is connected.", deviceName_);
                } else {
                    LOG_F(INFO, "{} is disconnected.", deviceName_);
                }
            },
            INDI::BaseDevice::WATCH_UPDATE);

        device.watchProperty(
            "DRIVER_INFO",
            [this](const INDI::PropertyText &property) {
                if (property.isValid()) {
                    const auto *driverName = property[0].getText();
                    LOG_F(INFO, "Driver name: {}", driverName);

                    const auto *driverExec = property[1].getText();
                    LOG_F(INFO, "Driver executable: {}", driverExec);
                    driverExec_ = driverExec;
                    const auto *driverVersion = property[2].getText();
                    LOG_F(INFO, "Driver version: {}", driverVersion);
                    driverVersion_ = driverVersion;
                    const auto *driverInterface = property[3].getText();
                    LOG_F(INFO, "Driver interface: {}", driverInterface);
                    driverInterface_ = driverInterface;
                }
            },
            INDI::BaseDevice::WATCH_NEW);

        device.watchProperty(
            "DEBUG",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    isDebug_.store(property[0].getState() == ISS_ON);
                    LOG_F(INFO, "Debug is {}", isDebug_.load() ? "ON" : "OFF");
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        // Max: 这个参数其实挺重要的，但是除了行星相机都不需要调整，默认就好
        device.watchProperty(
            "POLLING_PERIOD",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto period = property[0].getValue();
                    LOG_F(INFO, "Current polling period: {}", period);
                    if (period != currentPollingPeriod_.load()) {
                        LOG_F(INFO, "Polling period change to: {}", period);
                        currentPollingPeriod_ = period;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "DEVICE_AUTO_SEARCH",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    deviceAutoSearch_ = property[0].getState() == ISS_ON;
                    LOG_F(INFO, "Auto search is {}",
                          deviceAutoSearch_ ? "ON" : "OFF");
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "CONNECTION_MODE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    auto connectionMode = property[0].getState();
                    if (connectionMode == ISS_ON) {
                        LOG_F(INFO, "Connection mode is ON");
                        connectionMode_ = ConnectionMode::SERIAL;
                    } else if (connectionMode == ISS_OFF) {
                        LOG_F(INFO, "Connection mode is OFF");
                        connectionMode_ = ConnectionMode::TCP;
                    } else {
                        LOG_F(ERROR, "Unknown connection mode");
                        connectionMode_ = ConnectionMode::NONE;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "BAUD_RATE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    for (int i = 0; i < property.size(); i++) {
                        if (property[i].getState() == ISS_ON) {
                            LOG_F(INFO, "Baud rate is {}",
                                  property[i].getLabel());
                            baudRate_ = static_cast<BAUD_RATE>(i);
                        }
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "DEVICE_PORT_SCAN",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    devicePortScan_ = property[0].getState() == ISS_ON;
                    LOG_F(INFO, "Device port scan is {}",
                          devicePortScan_ ? "On" : "Off");
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "ACTIVE_DEVICES",
            [this](const INDI::PropertyText &property) {
                if (property.isValid()) {
                    if (property[0].getText() != nullptr) {
                        const auto *gps = property[0].getText();
                        LOG_F(INFO, "Active devices: {}", gps);
                        gps_ = getDevice(gps);
                    }
                    if (property[1].getText() != nullptr) {
                        const auto *dome = property[1].getText();
                        LOG_F(INFO, "Active devices: {}", dome);
                        dome_ = getDevice(dome);
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_TRACK_STATE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    isTracking_ = property[0].getState() == ISS_ON;
                    LOG_F(INFO, "Tracking state is {}",
                          isTracking_.load() ? "On" : "Off");
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_TRACK_MODE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    for (int i = 0; i < property.size(); i++) {
                        if (property[i].getState() == ISS_ON) {
                            LOG_F(INFO, "Track mode is {}",
                                  property[i].getLabel());
                            trackMode_ = static_cast<TrackMode>(i);
                        }
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_TRACK_RATE",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    trackRateRA_ = property[0].getValue();
                    trackRateDEC_ = property[1].getValue();
                    LOG_F(INFO, "Track rate RA: {}", trackRateRA_);
                    LOG_F(INFO, "Track rate DEC: {}", trackRateDEC_);
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_INFO",
            [this](const INDI::PropertyNumber &property) {
                {
                    if (property.isValid()) {
                        telescopeAperture_ = property[0].getValue();
                        LOG_F(INFO, "Telescope aperture: {}",
                              telescopeAperture_);
                        telescopeFocalLength_ = property[1].getValue();
                        LOG_F(INFO, "Telescope focal length: {}",
                              telescopeFocalLength_);
                        telescopeGuiderAperture_ = property[2].getValue();
                        LOG_F(INFO, "Telescope guider aperture: {}",
                              telescopeGuiderAperture_);
                        telescopeGuiderFocalLength_ = property[3].getValue();
                        LOG_F(INFO, "Telescope guider focal length: {}",
                              telescopeGuiderFocalLength_);
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_PIER_SIDE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        LOG_F(INFO, "Telescope pier side: EAST");
                        pierSide_ = PierSide::EAST;
                    } else if (property[1].getState() == ISS_ON) {
                        LOG_F(INFO, "Telescope pier side: WEST");
                        pierSide_ = PierSide::WEST;
                    } else {
                        LOG_F(INFO, "Telescope pier side: NONE");
                        pierSide_ = PierSide::NONE;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_PARK",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    isParked_ = property[0].getState() == ISS_ON;
                    LOG_F(INFO, "Park state: {}",
                          isParked_.load() ? "parked" : "unparked");
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);
        device_.watchProperty(
            "TELESCOPE_PARK_POSITION",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    telescopeParkPositionRA_ = property[0].getValue();
                    LOG_F(INFO, "Park position RA: {}",
                          telescopeParkPositionRA_);
                    telescopeParkPositionDEC_ = property[1].getValue();
                    LOG_F(INFO, "Park position DEC: {}",
                          telescopeParkPositionDEC_);
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_PARK_OPTION",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    for (int i = 0; i < property.size(); i++) {
                        if (i == property.size() - 1) {
                            parkOption_ = ParkOptions::NONE;
                        }
                        if (property[i].getState() == ISS_ON) {
                            LOG_F(INFO, "Park option is {}",
                                  property[i].getLabel());
                            parkOption_ = static_cast<ParkOptions>(i);
                        }
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "USEJOYSTICK",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    isJoystickEnabled_ = property[0].getState() == ISS_ON;
                    LOG_F(INFO, "Joystick is {}",
                          isJoystickEnabled_ ? "on" : "off");
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "SNOOP_JOYSTICK",
            [this](const INDI::PropertyText &property) {
                if (property.isValid()) {
                    if (isJoystickEnabled_) {
                        joystick_ = getDevice(property[0].getText());
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_SLEW_RATE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    totalSlewRate_ = property.size();
                    for (int i = 0; i < property.size(); i++) {
                        if (i == property.size() - 1) {
                            slewRate_ = SlewRate::NONE;
                        }
                        if (property[i].getState() == ISS_ON) {
                            LOG_F(INFO, "Slew rate is {}",
                                  property[i].getLabel());
                            slewRate_ = static_cast<SlewRate>(i);
                        }
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_MOTION_WE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        motionEW_ = MotionEW::WEST;
                    } else if (property[1].getState() == ISS_ON) {
                        motionEW_ = MotionEW::EAST;
                    } else {
                        motionEW_ = MotionEW::NONE;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_MOTION_NS",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        motionNS_ = MotionNS::NORTH;
                    } else if (property[1].getState() == ISS_ON) {
                        motionNS_ = MotionNS::SOUTH;
                    } else {
                        motionNS_ = MotionNS::NONE;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TELESCOPE_REVERSE_MOTION",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    motionNSReserved_ = property[0].getState() == ISS_ON;
                    motionEWReserved_ = property[1].getState() == ISS_ON;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "TARGET_EOD_COORD",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    targetSlewRA_ = property[0].getValue();
                    targetSlewDEC_ = property[1].getValue();
                    LOG_F(INFO, "Target slew RA: {}", targetSlewRA_);
                    LOG_F(INFO, "Target slew DEC: {}", targetSlewDEC_);
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "DOME_POLICY",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        domePolicy_ = DomePolicy::IGNORED;
                    } else if (property[1].getState() == ISS_ON) {
                        domePolicy_ = DomePolicy::LOCKED;
                    } else {
                        domePolicy_ = DomePolicy::NONE;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);
    });

    return true;
}
auto INDITelescope::disconnect() -> void {}
auto INDITelescope::reconnect() -> bool {}

auto INDITelescope::watchAdditionalProperty() -> bool {}

void INDITelescope::setPropertyNumber(std::string_view propertyName,
                                      double value) {}

auto INDITelescope::getTelescopeInfo()
    -> std::optional<std::tuple<double, double, double, double>> {
    INDI::PropertyNumber property = device_.getProperty("TELESCOPE_INFO");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_INFO property...");
        return std::nullopt;
    }
    telescopeAperture_ = property[0].getValue();
    telescopeFocalLength_ = property[1].getValue();
    telescopeGuiderAperture_ = property[2].getValue();
    telescopeGuiderFocalLength_ = property[3].getValue();
    return std::make_tuple(telescopeAperture_, telescopeFocalLength_,
                           telescopeGuiderAperture_,
                           telescopeGuiderFocalLength_);
}

auto INDITelescope::setTelescopeInfo(double telescopeAperture,
                                     double telescopeFocal,
                                     double guiderAperture,
                                     double guiderFocal) -> bool {
    INDI::PropertyNumber property = device_.getProperty("TELESCOPE_INFO");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_INFO property...");
        return false;
    }
    property[0].setValue(telescopeAperture);
    property[1].setValue(telescopeFocal);
    property[2].setValue(guiderAperture);
    property[3].setValue(guiderFocal);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopePierSide() -> std::optional<PierSide> {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_PIER_SIDE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_PIER_SIDE property...");
        return std::nullopt;
    }
    if (property[0].getState() == ISS_ON)
        return PierSide::EAST;
    else if (property[1].getState() == ISS_ON)
        return PierSide::WEST;
    return PierSide::NONE;
}

auto INDITelescope::getTelescopeTrackRate() -> std::optional<TrackMode> {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_TRACK_RATE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_TRACK_RATE property...");
        return std::nullopt;
    }
    if (property[0].getState() == ISS_ON)
        return TrackMode::SIDEREAL;
    else if (property[1].getState() == ISS_ON)
        return TrackMode::SOLAR;
    else if (property[2].getState() == ISS_ON)
        return TrackMode::LUNAR;
    else if (property[3].getState() == ISS_ON)
        return TrackMode::CUSTOM;
    return TrackMode::NONE;
}
auto INDITelescope::setTelescopeTrackRate(TrackMode rate) -> bool {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_TRACK_RATE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_TRACK_RATE property...");
        return false;
    }
    if (rate == TrackMode::SIDEREAL) {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
        property[2].setState(ISS_OFF);
        property[3].setState(ISS_OFF);
    } else if (rate == TrackMode::SOLAR) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
        property[2].setState(ISS_OFF);
        property[3].setState(ISS_OFF);
    } else if (rate == TrackMode::LUNAR) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_OFF);
        property[2].setState(ISS_ON);
        property[3].setState(ISS_OFF);
    } else if (rate == TrackMode::CUSTOM) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_OFF);
        property[2].setState(ISS_OFF);
        property[3].setState(ISS_ON);
    }
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeTrackEnable() -> bool {
    INDI::PropertySwitch property =
        device_.getProperty("TELESCOPE_TRACK_STATE");
    if (!property.isValid()) {
        isTrackingEnabled_ = false;
        LOG_F(ERROR, "Unable to find TELESCOPE_TRACK_STATE property...");
        return false;
    }
    return property[0].getState() == ISS_ON;
}

auto INDITelescope::setTelescopeTrackEnable(bool enable) -> bool {
    if (!isTrackingEnabled_) {
        LOG_F(ERROR, "Tracking is not enabled...");
        return false;
    }
    INDI::PropertySwitch property =
        device_.getProperty("TELESCOPE_TRACK_STATE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_TRACK_STATE property...");
        return false;
    }
    property[0].setState(enable ? ISS_ON : ISS_OFF);
    property[1].setState(enable ? ISS_OFF : ISS_ON);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::setTelescopeAbortMotion() -> bool {
    INDI::PropertySwitch property =
        device_.getProperty("TELESCOPE_ABORT_MOTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_ABORT_MOTION property...");
        return false;
    }
    property[0].setState(ISS_ON);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::setTelescopeParkOption(ParkOptions option) -> bool {
    INDI::PropertySwitch property =
        device_.getProperty("TELESCOPE_PARK_OPTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_PARK_OPTION property...");
        return false;
    }
    if (option == ParkOptions::CURRENT)
        property[0].setState(ISS_ON);
    else if (option == ParkOptions::DEFAULT)
        property[1].setState(ISS_ON);
    else if (option == ParkOptions::WRITE_DATA)
        property[2].setState(ISS_ON);
    else if (option == ParkOptions::PURGE_DATA)
        property[3].setState(ISS_ON);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeParkPosition()
    -> std::optional<std::pair<double, double>> {
    INDI::PropertyNumber property =
        device_.getProperty("TELESCOPE_PARK_POSITION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_PARK_POSITION property...");
        return std::nullopt;
    }
    return std::make_pair(property[0].getValue(), property[1].getValue());
}

auto INDITelescope::setTelescopeParkPosition(double parkRA,
                                             double parkDEC) -> bool {
    INDI::PropertyNumber property =
        device_.getProperty("TELESCOPE_PARK_POSITION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_PARK_POSITION property...");
        return false;
    }
    property[0].setValue(parkRA);
    property[1].setValue(parkDEC);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopePark() -> bool {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_PARK");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_PARK property...");
        return false;
    }
    return (property[0].getState() == ISS_ON);
}
auto INDITelescope::setTelescopePark(bool isParked) -> bool {
    if (!isParkEnabled_) {
        LOG_F(ERROR, "Parking is not enabled...");
        return false;
    }
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_PARK");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_PARK property...");
        return false;
    }
    property[0].setState(isParked ? ISS_ON : ISS_OFF);
    property[1].setState(isParked ? ISS_OFF : ISS_ON);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::setTelescopeHomeInit(std::string_view command) -> bool {
    INDI::PropertySwitch property = device_.getProperty("HOME_INIT");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find HOME_INIT property...");
        return false;
    }
    if (command == "SLEWHOME") {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
    } else if (command == "SYNCHOME") {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
    }
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeSlewRate() -> std::optional<double> {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_SLEW_RATE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_SLEW_RATE property...");
        return std::nullopt;
    }
    double speed = 0;
    for (int i = 0; i < property.count(); ++i) {
        if (property[i].getState() == ISS_ON) {
            speed = i;
            break;
        }
    }
    return speed;
}

auto INDITelescope::setTelescopeSlewRate(double speed) -> bool {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_SLEW_RATE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_SLEW_RATE property...");
        return false;
    }
    for (int i = 0; i < property.count(); ++i) {
        property[i].setState(i == speed ? ISS_ON : ISS_OFF);
    }
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeTotalSlewRate() -> std::optional<double> {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_SLEW_RATE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_SLEW_RATE property...");
        return std::nullopt;
    }
    return property.count();
}

auto INDITelescope::getTelescopeMoveWE() -> std::optional<MotionEW> {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_MOTION_WE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_MOTION_WE property...");
        return std::nullopt;
    }
    if (property[0].getState() == ISS_ON) {
        return MotionEW::EAST;
    }
    if (property[1].getState() == ISS_ON) {
        return MotionEW::WEST;
    }
    return MotionEW::NONE;
}

auto INDITelescope::setTelescopeMoveWE(MotionEW direction) -> bool {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_MOTION_WE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_MOTION_WE property...");
        return false;
    }
    if (direction == MotionEW::EAST) {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
    } else if (direction == MotionEW::WEST) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
    } else if (direction == MotionEW::NONE) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_OFF);
    }
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeMoveNS() -> std::optional<MotionNS> {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_MOTION_NS");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_MOTION_NS property...");
        return std::nullopt;
    }
    if (property[0].getState() == ISS_ON) {
        return MotionNS::NORTH;
    }
    if (property[1].getState() == ISS_ON) {
        return MotionNS::SOUTH;
    }
    return MotionNS::NONE;
}

auto INDITelescope::setTelescopeMoveNS(MotionNS direction) -> bool {
    INDI::PropertySwitch property = device_.getProperty("TELESCOPE_MOTION_NS");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_MOTION_NS property...");
        return false;
    }
    if (direction == MotionNS::NORTH) {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
    } else if (direction == MotionNS::SOUTH) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
    } else if (direction == MotionNS::NONE) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_OFF);
    }
    sendNewProperty(property);
    return true;
}

auto INDITelescope::setTelescopeGuideNS(int dir, int timeGuide) -> bool {
    INDI::PropertyNumber property =
        device_.getProperty("TELESCOPE_TIMED_GUIDE_NS");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_TIMED_GUIDE_NS property...");
        return false;
    }
    property[dir == 1 ? 1 : 0].setValue(timeGuide);
    property[dir == 1 ? 0 : 1].setValue(0);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::setTelescopeGuideWE(int dir, int timeGuide) -> bool {
    INDI::PropertyNumber property =
        device_.getProperty("TELESCOPE_TIMED_GUIDE_WE");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TELESCOPE_TIMED_GUIDE_WE property...");
        return false;
    }
    property[dir == 1 ? 1 : 0].setValue(timeGuide);
    property[dir == 1 ? 0 : 1].setValue(0);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::setTelescopeActionAfterPositionSet(std::string_view action)
    -> bool {
    INDI::PropertySwitch property = device_.getProperty("ON_COORD_SET");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find ON_COORD_SET property...");
        return false;
    }
    if (action == "STOP") {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
        property[2].setState(ISS_OFF);
    } else if (action == "TRACK") {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
        property[2].setState(ISS_OFF);
    } else if (action == "SYNC") {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_OFF);
        property[2].setState(ISS_ON);
    }
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeRADECJ2000()
    -> std::optional<std::pair<double, double>> {
    INDI::PropertyNumber property = device_.getProperty("EQUATORIAL_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find EQUATORIAL_COORD property...");
        return std::nullopt;
    }
    return std::make_pair(property[0].getValue(), property[1].getValue());
}

auto INDITelescope::setTelescopeRADECJ2000(double RAHours,
                                           double DECDegree) -> bool {
    INDI::PropertyNumber property = device_.getProperty("EQUATORIAL_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find EQUATORIAL_COORD property...");
        return false;
    }
    property[0].setValue(RAHours);
    property[1].setValue(DECDegree);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeRADECJNOW()
    -> std::optional<std::pair<double, double>> {
    INDI::PropertyNumber property = device_.getProperty("EQUATORIAL_EOD_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find EQUATORIAL_EOD_COORD property...");
        return std::nullopt;
    }
    return std::make_pair(property[0].getValue(), property[1].getValue());
}

auto INDITelescope::setTelescopeRADECJNOW(double RAHours,
                                          double DECDegree) -> bool {
    INDI::PropertyNumber property = device_.getProperty("EQUATORIAL_EOD_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find EQUATORIAL_EOD_COORD property...");
        return false;
    }
    property[0].setValue(RAHours);
    property[1].setValue(DECDegree);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::getTelescopeTargetRADECJNOW()
    -> std::optional<std::pair<double, double>> {
    INDI::PropertyNumber property = device_.getProperty("TARGET_EOD_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TARGET_EOD_COORD property...");
        return std::nullopt;
    }
    return std::make_pair(property[0].getValue(), property[1].getValue());
}

auto INDITelescope::setTelescopeTargetRADECJNOW(double RAHours,
                                                double DECDegree) -> bool {
    INDI::PropertyNumber property = device_.getProperty("TARGET_EOD_COORD");

    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find TARGET_EOD_COORD property...");
        return false;
    }
    property[0].setValue(RAHours);
    property[1].setValue(DECDegree);
    sendNewProperty(property);
    return true;
}

auto INDITelescope::slewTelescopeJNowNonBlock(double RAHours, double DECDegree,
                                              bool EnableTracking) -> bool {
    std::string action = EnableTracking ? "TRACK" : "STOP";
    setTelescopeActionAfterPositionSet(action);
    return setTelescopeRADECJNOW(RAHours, DECDegree);
}

auto INDITelescope::syncTelescopeJNow(double RAHours,
                                      double DECDegree) -> bool {
    setTelescopeActionAfterPositionSet("SYNC");
    return setTelescopeRADECJNOW(RAHours, DECDegree);
}

auto INDITelescope::getTelescopetAZALT()
    -> std::optional<std::pair<double, double>> {
    INDI::PropertyNumber property = device_.getProperty("HORIZONTAL_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find HORIZONTAL_COORD property...");
        return std::nullopt;
    }
    return std::make_pair(property[0].getValue(), property[1].getValue());
}

auto INDITelescope::setTelescopetAZALT(double AZ_DEGREE,
                                       double ALT_DEGREE) -> bool {
    INDI::PropertyNumber property = device_.getProperty("HORIZONTAL_COORD");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find HORIZONTAL_COORD property...");
        return false;
    }
    property[0].setValue(AZ_DEGREE);
    property[1].setValue(ALT_DEGREE);
    sendNewProperty(property);
    return true;
}
