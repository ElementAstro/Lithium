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
    /**
     * @brief 构造函数
     *
     * @param name 摄像机名称
     */
    explicit AtomCamera(const std::string &name);

    virtual ~AtomCamera();

    virtual auto connect(const std::string &deviceName, int timeout,
                         int maxRetry) -> bool;

    virtual auto disconnect(bool force, int timeout, int maxRetry) -> bool;

    virtual auto reconnect(int timeout, int maxRetry) -> bool;

    virtual auto scan() -> std::vector<std::string>;

    virtual auto isConnected() -> bool;

    virtual auto startExposure(const double &duration) -> bool;

    virtual auto abortExposure() -> bool;

    virtual auto getExposureStatus() -> bool;

    virtual auto getExposureResult() -> bool;

    virtual auto saveExposureResult() -> bool;

    virtual auto startVideo() -> bool;

    virtual auto stopVideo() -> bool;

    virtual auto getVideoStatus() -> bool;

    virtual auto getVideoResult() -> bool;

    virtual auto saveVideoResult() -> bool;

    virtual auto startCooling() -> bool;

    virtual auto stopCooling() -> bool;

    virtual auto getCoolingStatus() -> bool;

    virtual auto isCoolingAvailable() -> bool;

    virtual auto getTemperature() -> std::optional<double>;

    virtual auto getCoolingPower() -> bool;

    virtual auto setTemperature(const double &temperature) -> bool;

    virtual auto setCoolingPower(const double &power) -> bool;

    virtual auto getGain() -> std::optional<double>;

    virtual auto setGain(const int &gain) -> bool;

    virtual auto isGainAvailable() -> bool;

    virtual auto getOffset() -> std::optional<double>;

    virtual auto setOffset(const int &offset) -> bool;

    virtual auto isOffsetAvailable() -> bool;

    virtual auto getISO() -> bool;

    virtual auto setISO(const int &iso) -> bool;

    virtual auto isISOAvailable() -> bool;

    virtual auto getFrame() -> bool;

    virtual auto setFrame(const int &x, const int &y, const int &w,
                          const int &h) -> bool;

    virtual auto isFrameSettingAvailable() -> bool;

    virtual auto getBinning() -> std::optional<std::tuple<int, int, int, int>>;

    virtual auto setBinning(const int &hor, const int &ver) -> bool;

    virtual auto getFrameType() -> bool;

    virtual auto setFrameType(FrameType type) -> bool;

    virtual auto getUploadMode() -> bool;

    virtual auto setUploadMode(UploadMode mode) -> bool;
};
