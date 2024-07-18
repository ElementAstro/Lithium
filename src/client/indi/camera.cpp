// tutorial_client_base.cpp - 基类实现

#include "camera.hpp"
#include <libindi/abstractbaseclient.h>
#include <libindi/basedevice.h>
#include <libindi/indipropertynumber.h>
#include <libindi/indipropertyswitch.h>

#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include "atom/log/loguru.hpp"

INDICamera::INDICamera(std::string deviceName) : name_(std::move(deviceName)) {}

auto INDICamera::getDeviceInstance() -> INDI::BaseDevice & {
    if (!isConnected_.load()) {
        LOG_F(ERROR, "{} is not connected.", deviceName_);
        throw std::runtime_error("Device is not connected.");
    }
    return device_;
}

auto INDICamera::connect(const std::string &deviceName) -> bool {
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
                if (property[0].getState() == ISS_ON) {
                    LOG_F(INFO, "{} is connected.", deviceName_);
                    isConnected_.store(true);
                } else {
                    LOG_F(INFO, "{} is disconnected.", deviceName_);
                    isConnected_.store(false);
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
                    auto debugState = property[0].getState();
                    if (debugState == ISS_ON) {
                        LOG_F(INFO, "Debug is ON");
                        isDebug_.store(true);
                    } else if (debugState == ISS_OFF) {
                        LOG_F(INFO, "Debug is OFF");
                        isDebug_.store(false);
                    }
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

        device.watchProperty(
            "CCD_EXPOSURE",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto exposure = property[0].getValue();
                    LOG_F(INFO, "Current exposure time: {}", exposure);
                    currentExposure_ = exposure;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_TEMPERATURE",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto temp = property[0].getValue();
                    LOG_F(INFO, "Current temperature: {} C", temp);
                    currentTemperature_ = temp;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_COOLER",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    auto coolerState = property[0].getState();
                    if (coolerState == ISS_ON) {
                        LOG_F(INFO, "Cooler is ON");
                        isCooling_.store(true);
                    } else if (coolerState == ISS_OFF) {
                        LOG_F(INFO, "Cooler is OFF");
                        isCooling_.store(false);
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_TEMP_RAMP",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    auto slope = property[0].getValue();
                    auto threshold = property[1].getValue();
                    if (slope != currentSlope_.load()) {
                        LOG_F(INFO, "Max temperature slope change to: {}",
                              slope);
                        currentSlope_ = slope;
                    }
                    if (threshold != currentThreshold_.load()) {
                        LOG_F(INFO, "Max temperature threshold change to: {}",
                              threshold);

                        currentThreshold_ = threshold;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_GAIN",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    LOG_F(INFO, "Current gain: {}", property[0].getValue());
                    auto gain = property[0].getValue();
                    if (gain <= minGain_ || gain >= maxGain_) {
                        LOG_F(ERROR, "Gain out of range: {}", gain);
                    }
                    currentGain_ = gain;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_OFFSET",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    LOG_F(INFO, "Current offset: {}", property[0].getValue());
                    auto offset = property[0].getValue();
                    if (offset <= minGain_ || offset >= maxGain_) {
                        LOG_F(ERROR, "Gain out of range: {}", offset);
                    }
                    currentOffset_ = offset;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_FRAME",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    LOG_F(INFO, "Current frame X: {}", property[0].getValue());
                    frameX_ = property[0].getValue();
                    LOG_F(INFO, "Current frame Y: {}", property[1].getValue());
                    frameY_ = property[1].getValue();
                    LOG_F(INFO, "Current frame Width: {}",
                          property[2].getValue());
                    frameWidth_ = property[2].getValue();
                    LOG_F(INFO, "Current frame Height: {}",
                          property[3].getValue());
                    frameHeight_ = property[3].getValue();
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_BINNING",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    LOG_F(INFO, "Current binning X: {}",
                          property[0].getValue());
                    binHor_ = property[0].getValue();
                    LOG_F(INFO, "Current binning Y: {}",
                          property[1].getValue());
                    binVer_ = property[1].getValue();
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_TRANSFER_FORMAT",
            [this](const INDI::PropertySwitch &property) {
                if (property.isValid()) {
                    if (property[0].getState() == ISS_ON) {
                        LOG_F(INFO, "Transfer format is FITS");
                        imageFormat_ = ImageFormat::FITS;
                    } else if (property[1].getState() == ISS_ON) {
                        LOG_F(INFO, "Transfer format is NATIVE");
                        imageFormat_ = ImageFormat::NATIVE;
                    } else if (property[2].getState() == ISS_ON) {
                        LOG_F(INFO, "Transfer format is XISF");
                        imageFormat_ = ImageFormat::XISF;
                    } else {
                        LOG_F(ERROR, "Transfer format is NONE");
                        imageFormat_ = ImageFormat::NONE;
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        device.watchProperty(
            "CCD_INFO",
            [this](const INDI::PropertyNumber &property) {
                if (property.isValid()) {
                    LOG_F(INFO, "CCD_INFO: {}", device_.getDeviceName());
                    auto maxX = property[0].getValue();
                    LOG_F(INFO, "CCD maximum X pixel: {}", maxX);
                    maxFrameX_ = maxX;
                    auto maxY = property[1].getValue();
                    LOG_F(INFO, "CCD maximum Y pixel: {}", maxY);
                    maxFrameY_ = maxY;

                    auto framePixel = property[2].getValue();
                    LOG_F(INFO, "CCD frame pixel: {}", framePixel);
                    framePixel_ = framePixel;

                    auto framePixelX = property[3].getValue();
                    LOG_F(INFO, "CCD frame pixel X: {}", framePixelX);
                    framePixelX_ = framePixelX;

                    auto framePixelY = property[4].getValue();
                    LOG_F(INFO, "CCD frame pixel Y: {}", framePixelY);
                    framePixelY_ = framePixelY;

                    auto frameDepth = property[5].getValue();
                    LOG_F(INFO, "CCD frame depth: {}", frameDepth);
                    frameDepth_ = frameDepth;
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);

        // call if updated of the "CCD1" property - simplified way
        device.watchProperty(
            "CCD1",
            [](const INDI::PropertyBlob &property) {
                LOG_F(INFO, "Received image, size: {}",
                      property[0].getBlobLen());
                // Save FITS file to disk
                std::ofstream myfile;

                myfile.open("ccd_simulator.fits",
                            std::ios::out | std::ios::binary);
                myfile.write(static_cast<char *>(property[0].getBlob()),
                             property[0].getBlobLen());
                myfile.close();
                LOG_F(INFO, "Saved image to ccd_simulator.fits");
            },
            INDI::BaseDevice::WATCH_UPDATE);

        device.watchProperty(
            "ACTIVE_DEVICES",
            [this](const INDI::PropertyText &property) {
                if (property.isValid()) {
                    if (property[0].getText() != nullptr) {
                        telescope_ = getDevice(property[0].getText());
                    }
                    if (property[1].getText() != nullptr) {
                        rotator_ = getDevice(property[1].getText());
                    }
                    if (property[2].getText() != nullptr) {
                        focuser_ = getDevice(property[1].getText());
                    }
                    if (property[3].getText() != nullptr) {
                        filterwheel_ = getDevice(property[3].getText());
                    }
                }
            },
            INDI::BaseDevice::WATCH_NEW_OR_UPDATE);
    });
    return true;
}

auto INDICamera::disconnect() -> void {
    if (!isConnected_.load()) {
        LOG_F(ERROR, "{} is not connected.", deviceName_);
        return;
    }
    LOG_F(INFO, "Disconnecting from {}...", deviceName_);
    disconnectDevice(name_.c_str());
    LOG_F(INFO, "{} is disconnected.", deviceName_);
}

auto INDICamera::reconnect() -> bool {
    if (isConnected_.load()) {
        LOG_F(ERROR, "{} is already connected.", deviceName_);
        return false;
    }
    LOG_F(INFO, "Reconnecting to {}...", deviceName_);
    connectDevice(name_.c_str());
    LOG_F(INFO, "{} is reconnected.", deviceName_);
    return true;
}

auto INDICamera::watchAdditionalProperty() -> bool { return true; }

void INDICamera::setPropertyNumber(std::string_view propertyName,
                                   double value) {
    INDI::PropertyNumber property = device_.getProperty(propertyName.data());

    if (property.isValid()) {
        property[0].setValue(value);
        sendNewProperty(property);
    } else {
        LOG_F(ERROR, "Error: Unable to find property {}", propertyName);
    }
}

void INDICamera::newMessage(INDI::BaseDevice baseDevice, int messageID) {
    // Handle incoming messages from devices
}

auto INDICamera::startExposure(double exposure) -> bool {
    INDI::PropertyNumber exposureProperty =
        device_.getProperty("CCD_EXPOSURE");
    if (!exposureProperty.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_EXPOSURE property...");
        return false;
    }
    LOG_F(INFO, "Starting exposure of {} seconds...", exposure);
    exposureProperty[0].setValue(exposure);
    sendNewProperty(exposureProperty);
    return true;
}

auto INDICamera::abortExposure() -> bool {
    INDI::PropertySwitch ccdAbort = device_.getProperty("CCD_ABORT_EXPOSURE");
    if (!ccdAbort.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_ABORT_EXPOSURE property...");
        return false;
    }
    ccdAbort[0].setState(ISS_ON);
    sendNewProperty(ccdAbort);
    return true;
}

auto INDICamera::startCooling() -> bool { return setCooling(true); }

auto INDICamera::stopCooling() -> bool { return setCooling(false); }

auto INDICamera::setCooling(bool enable) -> bool {
    INDI::PropertySwitch ccdCooler = device_.getProperty("CCD_COOLER");
    if (!ccdCooler.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_COOLER property...");
        return false;
    }
    if (enable) {
        ccdCooler[0].setState(ISS_ON);
    } else {
        ccdCooler[0].setState(ISS_OFF);
    }
    sendNewProperty(ccdCooler);
    return true;
}

auto INDICamera::getTemperature() -> std::optional<double> {
    INDI::PropertyNumber ccdTemperature =
        device_.getProperty("CCD_TEMPERATURE");
    if (!ccdTemperature.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_TEMPERATURE property...");
        return std::nullopt;
    }
    currentTemperature_ = ccdTemperature[0].getValue();
    LOG_F(INFO, "Current temperature: {} C", currentTemperature_.load());
    return currentTemperature_;
}

auto INDICamera::setTemperature(double value) -> bool {
    if (!isConnected_.load()) {
        LOG_F(ERROR, "{} is not connected.", deviceName_);
        return false;
    }
    if (isExposing_.load()) {
        LOG_F(ERROR, "{} is exposing.", deviceName_);
        return false;
    }
    INDI::PropertyNumber ccdTemperature =
        device_.getProperty("CCD_TEMPERATURE");

    if (!ccdTemperature.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_TEMPERATURE property...");
        return false;
    }
    LOG_F(INFO, "Setting temperature to {} C...", value);
    ccdTemperature[0].setValue(value);
    sendNewProperty(ccdTemperature);
    return true;
}

auto INDICamera::getCameraFrameInfo()
    -> std::optional<std::tuple<int, int, int, int>> {
    INDI::PropertyNumber ccdFrameInfo = device_.getProperty("CCD_FRAME");

    if (!ccdFrameInfo.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_FRAME property...");
        return std::nullopt;
    }

    int x = ccdFrameInfo[0].getValue();
    int y = ccdFrameInfo[1].getValue();
    int width = ccdFrameInfo[2].getValue();
    int height = ccdFrameInfo[3].getValue();

    LOG_F(INFO, "CCD frame info: X: {}, Y: {}, WIDTH: {}, HEIGHT: {}", x, y,
          width, height);
    return std::make_tuple(x, y, width, height);
}

auto INDICamera::setCameraFrameInfo(int x, int y, int width,
                                    int height) -> bool {
    INDI::PropertyNumber ccdFrameInfo = device_.getProperty("CCD_FRAME");
    if (!ccdFrameInfo.isValid()) {
        LOG_F(ERROR,
              "Error: unable to find CCD Simulator ccdFrameInfo property");
        return false;
    }
    LOG_F(INFO, "setCameraFrameInfo {} {} {} {}", x, y, width, height);
    ccdFrameInfo[0].setValue(x);
    ccdFrameInfo[1].setValue(y);
    ccdFrameInfo[2].setValue(width);
    ccdFrameInfo[3].setValue(height);
    sendNewProperty(ccdFrameInfo);
    return true;
}

auto INDICamera::resetCameraFrameInfo() -> bool {
    INDI::PropertySwitch resetFrameInfo =
        device_.getProperty("CCD_FRAME_RESET");
    if (!resetFrameInfo.isValid()) {
        LOG_F(ERROR, "Error: unable to find resetCCDFrameInfo property...");
        return false;
    }
    resetFrameInfo[0].setState(ISS_ON);
    sendNewProperty(resetFrameInfo);
    resetFrameInfo[0].setState(ISS_OFF);
    sendNewProperty(resetFrameInfo);
    LOG_F(INFO, "Camera frame settings reset successfully");
    return true;
}

auto INDICamera::getGain() -> std::optional<double> {
    INDI::PropertyNumber ccdGain = device_.getProperty("CCD_GAIN");

    if (!ccdGain.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_GAIN property...");
        return std::nullopt;
    }

    currentGain_ = ccdGain[0].getValue();
    maxGain_ = ccdGain[0].getMax();
    minGain_ = ccdGain[0].getMin();
    return currentGain_;
}

auto INDICamera::setGain(double value) -> bool {
    INDI::PropertyNumber ccdGain = device_.getProperty("CCD_GAIN");

    if (!ccdGain.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_GAIN property...");
        return false;
    }
    LOG_F(INFO, "Setting gain to {}...", value);
    ccdGain[0].setValue(value);
    sendNewProperty(ccdGain);
    return true;
}

auto INDICamera::getOffset() -> std::optional<double> {
    INDI::PropertyNumber ccdOffset = device_.getProperty("CCD_OFFSET");

    if (!ccdOffset.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_OFFSET property...");
        return std::nullopt;
    }

    currentOffset_ = ccdOffset[0].getValue();
    maxOffset_ = ccdOffset[0].getMax();
    minOffset_ = ccdOffset[0].getMin();
    return currentOffset_;
}

auto INDICamera::setOffset(double value) -> bool {
    INDI::PropertyNumber ccdOffset = device_.getProperty("CCD_OFFSET");

    if (!ccdOffset.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_OFFSET property...");
        return false;
    }
    LOG_F(INFO, "Setting offset to {}...", value);
    ccdOffset[0].setValue(value);
    sendNewProperty(ccdOffset);
    return true;
}

auto INDICamera::getBinning() -> std::optional<std::tuple<int, int, int, int>> {
    INDI::PropertyNumber ccdBinning = device_.getProperty("CCD_BINNING");

    if (!ccdBinning.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_BINNING property...");
        return std::nullopt;
    }

    binHor_ = ccdBinning[0].getValue();
    binVer_ = ccdBinning[1].getValue();
    maxBinHor_ = ccdBinning[0].getMax();
    maxBinVer_ = ccdBinning[1].getMax();
    LOG_F(INFO, "Camera binning: {} x {}", binHor_, binVer_);
    return std::make_tuple(binHor_, binVer_, maxBinHor_, maxBinVer_);
}

auto INDICamera::setBinning(int binx, int biny) -> bool {
    INDI::PropertyNumber ccdBinning = device_.getProperty("CCD_BINNING");

    if (!ccdBinning.isValid()) {
        LOG_F(ERROR, "Error: unable to find CCD_BINNING property...");
        return false;
    }
    if (binx > maxBinHor_ || biny > maxBinVer_) {
        LOG_F(ERROR, "Error: binning value is out of range...");
        return false;
    }

    ccdBinning[0].setValue(binx);
    ccdBinning[1].setValue(biny);
    sendNewProperty(ccdBinning);
    LOG_F(INFO, "setCCDBinnign: {}, {}", binx, biny);
    return true;
}
