#ifndef ATOM_ASI_COMPONENT_HPP
#define ATOM_ASI_COMPONENT_HPP

#include "atom/driver/camera.hpp"

#include "driverlibs/libasi/ASICamera2.h"

#include <atomic>

class ASICamera : public AtomCamera {
public:
    explicit ASICamera(const std::string &name);
    ~ASICamera();

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