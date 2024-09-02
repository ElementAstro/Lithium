#include "filterwheel.hpp"

#include <optional>
#include <thread>
#include <tuple>

#include "atom/log/loguru.hpp"
#include "atom/utils/qtimer.hpp"

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

INDIFilterwheel::INDIFilterwheel(std::string name) : AtomFilterWheel(name) {}

auto INDIFilterwheel::connect(const std::string &deviceName, int timeout,
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
            "FILTER_SLOT",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    LOG_F(INFO, "Current filter slot: {}",
                          property[0].getValue());
                    currentSlot_ = property[0].getValue();
                    maxSlot_ = property[0].getMax();
                    minSlot_ = property[0].getMin();
                    currentSlotName_ =
                        slotNames_[static_cast<int>(property[0].getValue())];
                    LOG_F(INFO, "Current filter slot name: {}",
                          currentSlotName_);
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device_.watchProperty(
            "FILTER_NAME",
            [this](const INDI::PropertyText &property) {
                if (property.isValid()) {
                    slotNames_.clear();
                    for (const auto &filter : property) {
                        LOG_F(INFO, "Filter name: {}", filter.getText());
                        slotNames_.emplace_back(filter.getText());
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);
    });

    return true;
}
auto INDIFilterwheel::disconnect(bool force, int timeout,
                                 int maxRetry) -> bool {}
auto INDIFilterwheel::reconnect(int timeout, int maxRetry) -> bool {}

auto INDIFilterwheel::watchAdditionalProperty() -> bool {}

void INDIFilterwheel::setPropertyNumber(std::string_view propertyName,
                                        double value) {}

auto INDIFilterwheel::getCFWPosition()
    -> std::optional<std::tuple<double, double, double>> {
    INDI::PropertyNumber property = device_.getProperty("FILTER_SLOT");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find  FILTER_SLOT property...");
        return std::nullopt;
    }
    return std::make_tuple(property[0].getValue(), property[0].getMin(),
                           property[0].getMax());
}

auto INDIFilterwheel::setCFWPosition(int position) -> bool {
    INDI::PropertyNumber property = device_.getProperty("FILTER_SLOT");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find  FILTER_SLOT property...");
    }
    property[0].value = position;
    sendNewProperty(property);
    atom::utils::ElapsedTimer t;
    t.start();
    int timeout = 10000;
    while (t.elapsed() < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (property.getState() == IPS_OK) {
            break;  // it will not wait the motor arrived
        }
    }
    if (t.elapsed() > timeout) {
        LOG_F(ERROR, "setCFWPosition | ERROR : timeout ");
        return false;
    }
    return true;
}

auto INDIFilterwheel::getCFWSlotName() -> std::optional<std::string> {
    INDI::PropertyText property = device_.getProperty("FILTER_NAME");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find  FILTER_NAME property...");
        return std::nullopt;
    }
    return property[0].getText();
}

auto INDIFilterwheel::setCFWSlotName(std::string_view name) -> bool {
    INDI::PropertyText property = device_.getProperty("FILTER_NAME");
    if (!property.isValid()) {
        LOG_F(ERROR, "Unable to find  FILTER_NAME property...");
        return false;
    }
    property[0].setText(std::string(name).c_str());
    sendNewProperty(property);
    return true;
}

ATOM_MODULE(filterwheel_indi, [](Component &component) {
    LOG_F(INFO, "Registering filterwheel_indi module...");
    component.def("connect", &INDIFilterwheel::connect, "device",
                  "Connect to a filterwheel device.");
    component.def("disconnect", &INDIFilterwheel::disconnect, "device",
                  "Disconnect from a filterwheel device.");
    component.def("reconnect", &INDIFilterwheel::reconnect, "device",
                  "Reconnect to a filterwheel device.");
    component.def("scan", &INDIFilterwheel::scan,
                  "Scan for filterwheel devices.");
    component.def("is_connected", &INDIFilterwheel::isConnected,
                  "Check if a filterwheel device is connected.");

    component.def("initialize", &INDIFilterwheel::initialize, "device",
                  "Initialize a filterwheel device.");
    component.def("destroy", &INDIFilterwheel::destroy, "device",
                  "Destroy a filterwheel device.");

    component.def("get_position", &INDIFilterwheel::getCFWPosition,
                  "device", "Get the current filter position.");
    component.def("set_position", &INDIFilterwheel::setCFWPosition,
                  "device", "Set the current filter position.");
    component.def("get_slot_name", &INDIFilterwheel::getCFWSlotName,
                  "device", "Get the current filter slot name.");
    component.def("set_slot_name", &INDIFilterwheel::setCFWSlotName,
                  "device", "Set the current filter slot name.");

    component.def(
        "create_instance",
        [](const std::string &name) {
            std::shared_ptr<AtomFilterWheel> instance =
                std::make_shared<INDIFilterwheel>(name);
            return instance;
        },
        "device", "Create a new filterwheel instance.");
    component.defType<INDIFilterwheel>("filterwheel_indi", "device",
                                       "Define a new filterwheel instance.");

    LOG_F(INFO, "Registered filterwheel_indi module.");
});
