#ifndef ATOM_ASI_COMPONENT_HPP
#define ATOM_ASI_COMPONENT_HPP

#include "atom/driver/camera.hpp"

#include "driverlibs/libasi/ASICamera2.h"

#include <atomic>

class ASICamera : public AtomCamera {
public:
    explicit ASICamera(const std::string &name);
    ~ASICamera();

    bool initialize() final;
    bool destroy() final;

    bool connect(const json &params) final;

    bool disconnect(const json &params) final;

    bool reconnect(const json &params) final;

    bool isConnected() final;

    bool startExposure(const double &duration) final;

    bool abortExposure() final;

    bool getExposureStatus() final;

    bool getExposureResult() final;

    bool saveExposureResult() final;

    bool startVideo() final;

    bool stopVideo() final;

    bool getVideoStatus() final;

    bool getVideoResult() final;

    bool saveVideoResult() final;

    bool startCooling() final;

    bool stopCooling() final;

    bool getCoolingStatus() final;

    bool isCoolingAvailable() final;

    bool getTemperature() final;

    bool getCoolingPower() final;

    bool setTemperature(const double &temperature) final;

    bool setCoolingPower(const double &power) final;

    bool getGain() final;

    bool setGain(const int &gain) final;

    bool isGainAvailable() final;

    bool getOffset() final;

    bool setOffset(const int &offset) final;

    bool isOffsetAvailable() final;

    bool getISO() final;

    bool setISO(const int &iso) final;

    bool isISOAvailable() final;

    bool getFrame() final;

    bool setFrame(const int &x, const int &y, const int &w, const int &h) final;

    bool isFrameSettingAvailable() final;

    bool getBinning() final;

    bool setBinning(const int &hor, const int &ver) final;

    bool getFrameType() final;

    bool setFrameType(FrameType type) final;

    bool getUploadMode() final;

    bool setUploadMode(UploadMode mode) final;

private:
    bool refreshCameraInfo();

    /*ASI相机参数*/
    ASI_CAMERA_INFO ASICameraInfo;
    ASI_ERROR_CODE errCode;
    ASI_EXPOSURE_STATUS expStatus;

    int m_camera_id;
    std::string m_camera_name;

    std::atomic_bool is_connected;
    std::atomic_bool is_exposing;
    std::atomic_bool is_videoing;
    std::atomic_bool is_cooling;

    bool is_cooling_available;

    std::atomic<int> m_gain;
    std::atomic<int> m_offset;
};

#endif