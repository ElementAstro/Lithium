#ifndef LITHIUM_CLIENT_INDI_CAMERA_HPP
#define LITHIUM_CLIENT_INDI_CAMERA_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include <atomic>
#include <optional>
#include <string>

#include "device/template/camera.hpp"

enum class ImageFormat { FITS, NATIVE, XISF, NONE };

enum class CameraState {
    IDLE,
    EXPOSING,
    DOWNLOADING,
    IDLE_DOWNLOADING,
    ABORTED,
    ERROR,
    UNKNOWN
};

class INDICamera : public INDI::BaseClient, public AtomCamera {
public:
    explicit INDICamera(std::string name);
    ~INDICamera() override = default;

    auto initialize() -> bool override;

    auto destroy() -> bool override;

    auto connect(const std::string &deviceName, int timeout,
                 int maxRetry) -> bool override;

    auto disconnect(bool force, int timeout, int maxRetry) -> bool override;

    auto reconnect(int timeout, int maxRetry) -> bool override;

    auto scan() -> std::vector<std::string> override;

    auto isConnected() -> bool override;

    auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto startExposure(const double &exposure) -> bool override;
    auto abortExposure() -> bool override;
    auto getExposureStatus() -> bool override;
    auto getExposureResult() -> bool override;
    auto saveExposureResult() -> bool override;

    auto startVideo() -> bool override;
    auto stopVideo() -> bool override;
    auto getVideoResult() -> bool override;
    auto getVideoStatus() -> bool override;
    auto saveVideoResult() -> bool override;

    auto startCooling() -> bool override;
    auto stopCooling() -> bool override;
    auto getCoolingStatus() -> bool override;
    auto isCoolingAvailable() -> bool override;

    auto setTemperature(const double &value) -> bool override;
    auto getTemperature() -> std::optional<double> override;

    auto getCoolingPower() -> bool override;
    auto setCoolingPower(const double &value) -> bool override;

    auto getCameraFrameInfo() -> std::optional<std::tuple<int, int, int, int>>;
    auto setCameraFrameInfo(int x, int y, int width, int height) -> bool;
    auto resetCameraFrameInfo() -> bool;

    auto getGain() -> std::optional<double> override;
    auto setGain(const int &value) -> bool override;
    auto isGainAvailable() -> bool override;

    auto getOffset() -> std::optional<double> override;
    auto setOffset(const int &value) -> bool override;
    auto isOffsetAvailable() -> bool override;

    auto getISO() -> bool override;
    auto setISO(const int &iso) -> bool override;
    auto isISOAvailable() -> bool override;

    auto getFrame() -> std::optional<std::pair<int, int>> override;
    auto setFrame(const int &x, const int &y, const int &w,
                  const int &h) -> bool override;
    auto isFrameSettingAvailable() -> bool override;

    auto getFrameType() -> bool override;

    auto setFrameType(FrameType type) -> bool override;

    auto getUploadMode() -> bool override;

    auto setUploadMode(UploadMode mode) -> bool override;

    auto setBinning(const int &hor, const int &ver) -> bool override;
    auto getBinning() -> std::optional<std::tuple<int, int, int, int>> override;

    auto getDeviceInstance() -> INDI::BaseDevice &;

protected:
    void newMessage(INDI::BaseDevice baseDevice, int messageID) override;

private:
    auto setCooling(bool enable) -> bool;

    std::string name_;
    std::string deviceName_;

    std::string driverExec_;
    std::string driverVersion_;
    std::string driverInterface_;

    std::atomic<double> currentPollingPeriod_;

    std::atomic_bool isDebug_;

    std::atomic_bool isConnected_;

    std::atomic<double> currentExposure_;
    std::atomic_bool isExposing_;

    bool isCoolingEnable_;
    std::atomic_bool isCooling_;
    std::atomic<double> currentTemperature_;
    double maxTemperature_;
    double minTemperature_;
    std::atomic<double> currentSlope_;
    std::atomic<double> currentThreshold_;

    std::atomic<double> currentGain_;
    double maxGain_;
    double minGain_;

    std::atomic<double> currentOffset_;
    double maxOffset_;
    double minOffset_;

    double frameX_;
    double frameY_;
    double frameWidth_;
    double frameHeight_;
    double maxFrameX_;
    double maxFrameY_;

    double framePixel_;
    double framePixelX_;
    double framePixelY_;

    double frameDepth_;

    double binHor_;
    double binVer_;
    double maxBinHor_;
    double maxBinVer_;

    ImageFormat imageFormat_;

    INDI::BaseDevice device_;
    // Max: 相关的设备，也进行处理，可以联合操作
    INDI::BaseDevice telescope_;
    INDI::BaseDevice focuser_;
    INDI::BaseDevice rotator_;
    INDI::BaseDevice filterwheel_;
};

#endif
