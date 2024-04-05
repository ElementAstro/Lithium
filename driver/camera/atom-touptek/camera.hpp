#ifndef ATOM_TOUPTEK_COMPONENT_HPP
#define ATOM_TOUPTEK_COMPONENT_HPP

#include "atom/driver/camera.hpp"

#include "libtouptek/toupcam.h"

#include <atomic>

class ToupCamera : public AtomCamera {
public:
    explicit ToupCamera(const std::string &name);
    ~ToupCamera();

    bool initialize() override;
    bool destroy() override;

    bool connect(const json &params) override;

    bool disconnect(const json &params) override;

    bool reconnect(const json &params) override;

    bool isConnected() override;

    bool startExposure(const double &duration);

    bool abortExposure();

    bool getExposureStatus();

    bool getExposureResult();

    bool saveExposureResult();

    bool startVideo();

    bool stopVideo();

    bool getVideoStatus();

    bool getVideoResult();

    bool saveVideoResult();

    bool startCooling();

    bool stopCooling();

    bool getCoolingStatus();

    bool isCoolingAvailable();

    bool getTemperature();

    bool getCoolingPower();

    bool setTemperature(const double &temperature);

    bool setCoolingPower(const double &power);

    bool getGain();

    bool setGain(const int &gain);

    bool isGainAvailable();

    bool getOffset();

    bool setOffset(const int &offset);

    bool isOffsetAvailable();

    bool getISO();

    bool setISO(const int &iso);

    bool isISOAvailable();

    bool getFrame();

    bool setFrame(const int &x, const int &y, const int &w, const int &h);

    bool isFrameSettingAvailable();

    bool getBinning();

    bool setBinning(const int &hor, const int &ver);

    bool getFrameType();

    bool setFrameType(FrameType type);

    bool getUploadMode();

    bool setUploadMode(UploadMode mode);

private:
};

#endif