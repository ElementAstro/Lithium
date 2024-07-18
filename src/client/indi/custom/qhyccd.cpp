#include "qhyccd.hpp"

#include "atom/log/loguru.hpp"

QHYCamera::QHYCamera(std::string deviceName) : INDICamera(deviceName) {
    LOG_F(INFO, "QHYCamera::QHYCamera({})", deviceName.c_str());
}

QHYCamera::~QHYCamera() {
    LOG_F(INFO, "QHYCamera::~QHYCamera()");
}

auto QHYCamera::watchAdditionalProperty() -> bool {
    LOG_F(INFO, "QHYCamera::watchAdditionalProperty()");
    auto device = getDeviceInstance();
    device.watchProperty("READ_MODE", [this](const INDI::PropertyNumber &property){
        LOG_F(INFO, "QHYCamera::watchAdditionalProperty()::READ_MODE");
        if (property.isValid()) {
            auto value = property[0].getValue();
            LOG_F(INFO, "QHYCamera::watchAdditionalProperty()::READ_MODE::value={}", value);
            
        }
    });
    return true;
}