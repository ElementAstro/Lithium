/*
 * camera.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: AtomCamera Simulator and Basic Definition

*************************************************/

#pragma once

#include "device.hpp"

#ifdef ENABLE_SHARED_MEMORY
#include "shared_memory.hpp"
#endif

class AtomCameraFrame {
public:
    std::atomic_int binning_x;
    std::atomic_int binning_y;

    std::atomic<double> pixel;
    std::atomic<double> pixel_x;
    std::atomic<double> pixel_y;
    std::atomic<double> pixel_depth;

    std::atomic<double> frame_x;
    std::atomic<double> frame_y;
    std::atomic<double> max_frame_x;
    std::atomic<double> max_frame_y;

    std::atomic_int frame_height;
    std::atomic_int frame_width;

    std::string frame_type;
    std::string frame_format;
    std::string upload_mode;
    std::atomic_bool is_fastread;
};

enum class FrameType { FITS, NATIVE, XISF, JPG, PNG, TIFF };

enum class UploadMode { CLIENT, LOCAL, BOTH, CLOUD };
class AtomCamera : public AtomDriver {
public:
    /**
     * @brief 构造函数
     *
     * @param name 摄像机名称
     */
    explicit AtomCamera(const std::string &name);

    virtual ~AtomCamera();

    virtual bool initialize() override;

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

public:
    virtual bool startExposure(const double &duration);

    virtual bool abortExposure();

    virtual bool getExposureStatus();

    virtual bool getExposureResult();

    virtual bool saveExposureResult();

    virtual bool startVideo();

    virtual bool stopVideo();

    virtual bool getVideoStatus();

    virtual bool getVideoResult();

    virtual bool saveVideoResult();

    virtual bool startCooling();

    virtual bool stopCooling();

    virtual bool getCoolingStatus();

    virtual bool isCoolingAvailable();

    virtual bool getTemperature();

    virtual bool getCoolingPower();

    virtual bool setTemperature(const double &temperature);

    virtual bool setCoolingPower(const double &power);

    virtual bool getGain();

    virtual bool setGain(const int &gain);

    virtual bool isGainAvailable();

    virtual bool getOffset();

    virtual bool setOffset(const int &offset);

    virtual bool isOffsetAvailable();

    virtual bool getISO();

    virtual bool setISO(const int &iso);

    virtual bool isISOAvailable();

    virtual bool getFrame();

    virtual bool setFrame(const int &x, const int &y, const int &w,
                          const int &h);

    virtual bool isFrameSettingAvailable();

    virtual bool getBinning();

    virtual bool setBinning(const int &hor, const int &ver);

    virtual bool getFrameType();

    virtual bool setFrameType(FrameType type);

    virtual bool getUploadMode();

    virtual bool setUploadMode(UploadMode mode);

protected:
    json _startExposure(const json &params);

    json _abortExposure(const json &params);

    json _getExposureStatus(const json &params);

    json _getExposureResult(const json &params);

    json _saveExposureResult(const json &params);

    json _startVideo(const json &params);

    json _stopVideo(const json &params);

    json _getVideoStatus(const json &params);

    json _getVideoResult(const json &params);

    json _saveVideoResult(const json &params);

    json _startCooling(const json &params);

    json _stopCooling(const json &params);

    json _getCoolingStatus(const json &params);

    json _getTemperature(const json &params);

    json _getCoolingPower(const json &params);

    json _setTemperature(const json &params);

    json _setCoolingPower(const json &params);

    json _getGain(const json &params);

    json _setGain(const json &params);

    json _getOffset(const json &params);

    json _setOffset(const json &params);

    json _getISO(const json &params);

    json _setISO(const json &params);

    json _getFrame(const json &params);

    json _setFrame(const json &params);

    json _getBinning(const json &params);

    json _setBinning(const json &params);
};
