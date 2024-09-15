#pragma once

#include <memory>
#include <string>
#include <chrono>

class PHDSharedMemoryClient {
public:
    PHDSharedMemoryClient();
    ~PHDSharedMemoryClient();

    // Disable copy operations
    PHDSharedMemoryClient(const PHDSharedMemoryClient&) = delete;
    PHDSharedMemoryClient& operator=(const PHDSharedMemoryClient&) = delete;

    // Enable move operations
    PHDSharedMemoryClient(PHDSharedMemoryClient&&) noexcept;
    PHDSharedMemoryClient& operator=(PHDSharedMemoryClient&&) noexcept;

    bool connectPHD();
    bool call_phd_GetVersion(std::string& versionName);
    bool call_phd_StartLooping();
    bool call_phd_StopLooping();
    bool call_phd_AutoFindStar();
    bool call_phd_StartGuiding();
    bool call_phd_checkStatus(unsigned char& status);
    bool call_phd_setExposureTime(unsigned int expTime);
    bool call_phd_whichCamera(const std::string& camera);
    bool call_phd_ChackControlStatus(int sdk_num);
    bool call_phd_ClearCalibration();

    void showPHDData();
    void controlGuide(int direction, int duration);
    void getPHD2ControlInstruct();

    // New functions
    bool startCalibration();
    bool abortCalibration();
    bool dither(double pixels);
    bool setLockPosition(double x, double y);
    std::pair<double, double> getStarPosition() const;
    double getGuideRMSError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
