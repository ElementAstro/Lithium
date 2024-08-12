#include "focuser.hpp"

#include <optional>
#include <tuple>

#include "atom/log/loguru.hpp"

INDIFocuser::INDIFocuser(std::string name) : name_(name) {}

auto INDIFocuser::connect(const std::string &deviceName) -> bool {
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
    });

    return true;
}
auto INDIFocuser::disconnect() -> void {}
auto INDIFocuser::reconnect() -> bool {}

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

auto INDIFocuser::getFocuserMoveDiretion() -> bool {
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

auto INDIFocuser::setFocuserMoveDiretion(bool isDirectionIn) -> bool {
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
