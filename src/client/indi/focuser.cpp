#include "focuser.hpp"

#include <optional>
#include <tuple>

#include "atom/log/loguru.hpp"

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

INDIFocuser::INDIFocuser(std::string name) : AtomFocuser(name) {}

auto INDIFocuser::initialize() -> bool {
    return true;
}

auto INDIFocuser::destroy() -> bool {
    return true;
}

auto INDIFocuser::connect(const std::string &deviceName, int timeout,
                          int maxRetry) -> bool {
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
            "Mode",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    for (int i = 0; i < property.size(); i++) {
                        if (property[i].getState() == ISS_ON) {
                            LOG_F(INFO, "Focuser mode is {}",
                                  property[i].getLabel());
                            focusMode_ = static_cast<FocusMode>(i);
                        }
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_MOTION",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    for (int i = 0; i < property.size(); i++) {
                        if (property[i].getState() == ISS_ON) {
                            LOG_F(INFO, "Focuser motion is {}",
                                  property[i].getLabel());
                            focusDirection_ = static_cast<FocusDirection>(i);
                        }
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_SPEED",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto speed = property[0].getValue();
                    LOG_F(INFO, "Current focuser speed: {}", speed);
                    currentFocusSpeed_ = speed;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "REL_FOCUS_POSITION",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto position = property[0].getValue();
                    LOG_F(INFO, "Current relative focuser position: {}",
                          position);
                    realRelativePosition_ = position;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "ABS_FOCUS_POSITION",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto position = property[0].getValue();
                    LOG_F(INFO, "Current absolute focuser position: {}",
                          position);
                    realAbsolutePosition_ = position;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_MAX",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto maxlimit = property[0].getValue();
                    LOG_F(INFO, "Current focuser max limit: {}", maxlimit);
                    maxPosition_ = maxlimit;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_BACKLASH_TOGGLE",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        LOG_F(INFO, "Backlash is enabled");
                        backlashEnabled_ = true;
                    } else {
                        LOG_F(INFO, "Backlash is disabled");
                        backlashEnabled_ = false;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_BACKLASH_STEPS",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto backlash = property[0].getValue();
                    LOG_F(INFO, "Current focuser backlash: {}", backlash);
                    backlashSteps_ = backlash;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_TEMPERATURE",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto temperature = property[0].getValue();
                    LOG_F(INFO, "Current focuser temperature: {}", temperature);
                    temperature_ = temperature;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "CHIP_TEMPERATURE",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto temperature = property[0].getValue();
                    LOG_F(INFO, "Current chip temperature: {}", temperature);
                    chipTemperature_ = temperature;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "DELAY",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto delay = property[0].getValue();
                    LOG_F(INFO, "Current focuser delay: {}", delay);
                    delay_msec_ = delay;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "FOCUS_REVERSE_MOTION",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        LOG_F(INFO, "Focuser is reversed");
                        isReverse_ = true;
                    } else {
                        LOG_F(INFO, "Focuser is not reversed");
                        isReverse_ = false;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_TIMER",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto timer = property[0].getValue();
                    LOG_F(INFO, "Current focuser timer: {}", timer);
                    focusTimer_ = timer;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FOCUS_ABORT_MOTION",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        LOG_F(INFO, "Focuser is aborting");
                        isFocuserMoving_ = false;
                    } else {
                        LOG_F(INFO, "Focuser is not aborting");
                        isFocuserMoving_ = true;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);
    });

    return true;
}
auto INDIFocuser::disconnect(bool force, int timeout, int maxRetry) -> bool {}
auto INDIFocuser::reconnect(int timeout, int maxRetry) -> bool {}

auto INDIFocuser::watchAdditionalProperty() -> bool {}

void INDIFocuser::setPropertyNumber(std::string_view propertyName,
                                    double value) {}

auto INDIFocuser::getFocuserSpeed()
    -> std::optional<std::tuple<double, double, double>> {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_SPEED");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_SPEED property...");
        return std::nullopt;
    }
    return std::make_tuple(property[0].getValue(), property[1].getValue(),
                           property[2].getValue());
}

auto INDIFocuser::setFocuserSpeed(int value) -> bool {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_SPEED");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_SPEED property...");
        return false;
    }
    property[0].value = value;
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::getFocuserMoveDirection() -> bool {
    INDI::PropertySwitch property = device_.getProperty("FOCUS_MOTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_MOTION property...");
        return false;
    }
    if (property[0].getState() == ISS_ON) {
        return true;
    }
    return false;
}

auto INDIFocuser::setFocuserMoveDirection(bool isDirectionIn) -> bool {
    INDI::PropertySwitch property = device_.getProperty("FOCUS_MOTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_MOTION property...");
        return false;
    }
    if (isDirectionIn) {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
    }
    if (!isDirectionIn) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
    }
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::getFocuserMaxLimit() -> std::optional<int> {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_MAX");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_MAX property...");
        return std::nullopt;
    }
    return property[0].getValue();
}

auto INDIFocuser::setFocuserMaxLimit(int maxlimit) -> bool {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_MAX");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_MAX property...");
        return false;
    }
    property[0].value = maxlimit;
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::getFocuserReverse() -> std::optional<bool> {
    INDI::PropertySwitch property = device_.getProperty("FOCUS_REVERSE_MOTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_REVERSE_MOTION property...");
        return std::nullopt;
    }
    if (property[0].getState() == ISS_ON) {
        return true;
    }
    if (property[1].getState() == ISS_ON) {
        return false;
    }
    return std::nullopt;
}

auto INDIFocuser::setFocuserReverse(bool isReversed) -> bool {
    INDI::PropertySwitch property = device_.getProperty("FOCUS_REVERSE_MOTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_REVERSE_MOTION property...");
        return false;
    }
    if (isReversed) {
        property[0].setState(ISS_ON);
        property[1].setState(ISS_OFF);
    }
    if (!isReversed) {
        property[0].setState(ISS_OFF);
        property[1].setState(ISS_ON);
    }
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::moveFocuserSteps(int steps) -> bool {
    INDI::PropertyNumber property = device_.getProperty("REL_FOCUS_POSITION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find REL_FOCUS_POSITION property...");
        return false;
    }
    property[0].value = steps;
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::moveFocuserToAbsolutePosition(int position) -> bool {
    INDI::PropertyNumber property = device_.getProperty("ABS_FOCUS_POSITION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find ABS_FOCUS_POSITION property...");
        return false;
    }
    property[0].value = position;
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::getFocuserAbsolutePosition() -> std::optional<double> {
    INDI::PropertyNumber property = device_.getProperty("ABS_FOCUS_POSITION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find ABS_FOCUS_POSITION property...");
        return std::nullopt;
    }
    return property[0].getValue();
}

auto INDIFocuser::moveFocuserWithTime(int msec) -> bool {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_TIMER");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_TIMER property...");
        return false;
    }
    property[0].value = msec;
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::abortFocuserMove() -> bool {
    INDI::PropertySwitch property = device_.getProperty("FOCUS_ABORT_MOTION");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_ABORT_MOTION property...");
        return false;
    }
    property[0].setState(ISS_ON);
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::syncFocuserPosition(int position) -> bool {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_SYNC");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_SYNC property...");
        return false;
    }
    property[0].value = position;
    sendNewProperty(property);
    return true;
}

auto INDIFocuser::getFocuserOutTemperature() -> std::optional<double> {
    INDI::PropertyNumber property = device_.getProperty("FOCUS_TEMPERATURE");
    sendNewProperty(property);
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find FOCUS_TEMPERATURE property...");
        return std::nullopt;
    }
    return property[0].getValue();
}

auto INDIFocuser::getFocuserChipTemperature() -> std::optional<double> {
    INDI::PropertyNumber property = device_.getProperty("CHIP_TEMPERATURE");
    sendNewProperty(property);
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find CHIP_TEMPERATURE property...");
        return std::nullopt;
    }
    return property[0].getValue();
}

ATOM_MODULE(focuser_indi, [](Component &component) {
    LOG_F(INFO, "Registering focuser_indi module...");
    component.doc("INDI Focuser");
    component.def("initialize", &INDIFocuser::initialize, "device",
                  "Initialize a focuser device.");
    component.def("destroy", &INDIFocuser::destroy, "device",
                  "Destroy a focuser device.");
    component.def("connect", &INDIFocuser::connect, "device",
                  "Connect to a focuser device.");
    component.def("disconnect", &INDIFocuser::disconnect, "device",
                  "Disconnect from a focuser device.");
    component.def("reconnect", &INDIFocuser::reconnect, "device",
                  "Reconnect to a focuser device.");
    component.def("scan", &INDIFocuser::scan, "device",
                  "Scan for focuser devices.");
    component.def("is_connected", &INDIFocuser::isConnected, "device",
                  "Check if a focuser device is connected.");

    component.def("get_focuser_speed", &INDIFocuser::getFocuserSpeed, "device",
                  "Get the focuser speed.");
    component.def("set_focuser_speed", &INDIFocuser::setFocuserSpeed, "device",
                  "Set the focuser speed.");

    component.def("get_move_direction", &INDIFocuser::getFocuserMoveDirection,
                  "device", "Get the focuser mover direction.");
    component.def("set_move_direction", &INDIFocuser::setFocuserMoveDirection,
                  "device", "Set the focuser mover direction.");

    component.def("get_max_limit", &INDIFocuser::getFocuserMaxLimit, "device",
                  "Get the focuser max limit.");
    component.def("set_max_limit", &INDIFocuser::setFocuserMaxLimit, "device",
                  "Set the focuser max limit.");

    component.def("get_reverse", &INDIFocuser::getFocuserReverse, "device",
                  "Get whether the focuser reverse is enabled.");
    component.def("set_reverse", &INDIFocuser::setFocuserReverse, "device",
                  "Set whether the focuser reverse is enabled.");

    component.def("move_steps", &INDIFocuser::moveFocuserSteps, "device",
                  "Move the focuser steps.");
    component.def("move_to_absolute_position",
                  &INDIFocuser::moveFocuserToAbsolutePosition, "device",
                  "Move the focuser to absolute position.");
    component.def("get_absolute_position",
                  &INDIFocuser::getFocuserAbsolutePosition, "device",
                  "Get the focuser absolute position.");
    component.def("move_with_time", &INDIFocuser::moveFocuserWithTime, "device",
                  "Move the focuser with time.");
    component.def("abort_move", &INDIFocuser::abortFocuserMove, "device",
                  "Abort the focuser move.");
    component.def("sync_position", &INDIFocuser::syncFocuserPosition, "device",
                  "Sync the focuser position.");
    component.def("get_out_temperature", &INDIFocuser::getFocuserOutTemperature,
                  "device", "Get the focuser out temperature.");
    component.def("get_chip_temperature",
                  &INDIFocuser::getFocuserChipTemperature, "device",
                  "Get the focuser chip temperature.");

    component.def(
        "create_instance",
        [](const std::string &name) {
            std::shared_ptr<AtomFocuser> instance =
                std::make_shared<INDIFocuser>(name);
            return instance;
        },
        "device", "Create a new focuser instance.");
    component.defType<INDIFocuser>("focuser_indi", "device",
                                   "Define a new focuser instance.");

    LOG_F(INFO, "Registered focuser_indi module.");
});
