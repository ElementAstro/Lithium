#pragma once

#include "ascom_device.hpp"
#include "core/camera.hpp"

class ASCOMCamera : public Camera, public ASCOMDevice {
public:
    explicit ASCOMCamera(const std::string &name);
    ~ASCOMCamera();

public:
    virtual bool connect(const nlohmann::json &params) override;
    virtual bool disconnect(const nlohmann::json &params) override;
    virtual bool reconnect(const nlohmann::json &params) override;

public:
    virtual bool startExposure(const nlohmann::json &params) override;
    virtual bool abortExposure(const nlohmann::json &params) override;
    virtual bool getExposureStatus(const nlohmann::json &params) override;
    virtual bool getExposureResult(const nlohmann::json &params) override;
    virtual bool saveExposureResult(const nlohmann::json &params) override;
    virtual bool startVideo(const nlohmann::json &params) override;
    virtual bool stopVideo(const nlohmann::json &params) override;
    virtual bool getVideoStatus(const nlohmann::json &params) override;
    virtual bool getVideoResult(const nlohmann::json &params) override;
    virtual bool saveVideoResult(const nlohmann::json &params) override;
    virtual bool startCooling(const nlohmann::json &params) override;
    virtual bool stopCooling(const nlohmann::json &params) override;
    virtual bool getTemperature(const nlohmann::json &params) override;
    virtual bool getCoolingPower(const nlohmann::json &params) override;
    virtual bool setTemperature(const nlohmann::json &params) override;
    virtual bool setCoolingPower(const nlohmann::json &params) override;
    virtual bool getGain(const nlohmann::json &params) override;
    virtual bool setGain(const nlohmann::json &params) override;
    virtual bool getOffset(const nlohmann::json &params) override;
    virtual bool setOffset(const nlohmann::json &params) override;
    virtual bool getISO(const nlohmann::json &params) override;
    virtual bool setISO(const nlohmann::json &params) override;
    virtual bool getFrame(const nlohmann::json &params) override;
    virtual bool setFrame(const nlohmann::json &params) override;

public:
};