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

#include <atomic>
#include <optional>

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
    explicit AtomCamera(const std::string &name) : AtomDriver(name) {}

    virtual auto startExposure(const double &duration) -> bool = 0;

    virtual auto abortExposure() -> bool = 0;

    virtual auto getExposureStatus() -> bool = 0;

    virtual auto getExposureResult() -> bool = 0;

    virtual auto saveExposureResult() -> bool = 0;

    virtual auto startVideo() -> bool = 0;

    virtual auto stopVideo() -> bool = 0;

    virtual auto getVideoStatus() -> bool = 0;

    virtual auto getVideoResult() -> bool = 0;

    virtual auto saveVideoResult() -> bool = 0;

    virtual auto startCooling() -> bool = 0;

    virtual auto stopCooling() -> bool = 0;

    virtual auto getCoolingStatus() -> bool = 0;

    virtual auto isCoolingAvailable() -> bool;

    virtual auto getTemperature() -> std::optional<double> = 0;

    virtual auto getCoolingPower() -> bool = 0;

    virtual auto setTemperature(const double &temperature) -> bool = 0;

    virtual auto setCoolingPower(const double &power) -> bool = 0;

    virtual auto getGain() -> std::optional<double> = 0;

    virtual auto setGain(const int &gain) -> bool = 0;

    virtual auto isGainAvailable() -> bool = 0;

    virtual auto getOffset() -> std::optional<double> = 0;

    virtual auto setOffset(const int &offset) -> bool = 0;

    virtual auto isOffsetAvailable() -> bool = 0;

    virtual auto getISO() -> bool = 0;

    virtual auto setISO(const int &iso) -> bool = 0;

    virtual auto isISOAvailable() -> bool = 0;

    virtual auto getFrame() -> bool = 0;

    virtual auto setFrame(const int &x, const int &y, const int &w,
                          const int &h) -> bool = 0;

    virtual auto isFrameSettingAvailable() -> bool = 0;

    virtual auto getBinning()
        -> std::optional<std::tuple<int, int, int, int>> = 0;

    virtual auto setBinning(const int &hor, const int &ver) -> bool = 0;

    virtual auto getFrameType() -> bool = 0;

    virtual auto setFrameType(FrameType type) -> bool = 0;

    virtual auto getUploadMode() -> bool = 0;

    virtual auto setUploadMode(UploadMode mode) -> bool = 0;
};
