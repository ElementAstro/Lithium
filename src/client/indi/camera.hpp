#ifndef LITHIUM_CLIENT_INDI_CAMERA_HPP
#define LITHIUM_CLIENT_INDI_CAMERA_HPP

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

#include <atomic>
#include <optional>
#include <string>

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

class INDICamera : public INDI::BaseClient {
public:
    explicit INDICamera(std::string name);
    ~INDICamera() override = default;

    auto connect(const std::string &deviceName) -> bool;
    auto disconnect() -> void;
    auto reconnect() -> bool;

    virtual auto watchAdditionalProperty() -> bool;

    void setPropertyNumber(std::string_view propertyName, double value);

    auto startExposure(double exposure) -> bool;
    auto abortExposure() -> bool;

    auto startCooling() -> bool;
    auto stopCooling() -> bool;

    auto setTemperature(double value) -> bool;
    auto getTemperature() -> std::optional<double>;

    auto getCameraFrameInfo() -> std::optional<std::tuple<int, int, int, int>>;
    auto setCameraFrameInfo(int x, int y, int width, int height) -> bool;
    auto resetCameraFrameInfo() -> bool;

    auto getGain() -> std::optional<double>;
    auto setGain(double value) -> bool;
    auto getOffset() -> std::optional<double>;
    auto setOffset(double value) -> bool;

    auto setBinning(int binx, int biny) -> bool;
    auto getBinning() -> std::optional<std::tuple<int, int, int, int>>;

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