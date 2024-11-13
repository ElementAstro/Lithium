#include "task_camera.hpp"
#include <memory>

#include "config/configor.hpp"
#include "task/simple/task.hpp"

#include "device/template/camera.hpp"

#include "atom/async/timer.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/concept.hpp"
#include "atom/function/enum.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

#include "utils/constant.hpp"

using namespace atom::meta;

template <>
struct EnumTraits<lithium::sequencer::task::ExposureType> {
    static constexpr std::array<lithium::sequencer::task::ExposureType, 5>
        values = {lithium::sequencer::task::ExposureType::LIGHT,
                  lithium::sequencer::task::ExposureType::DARK,
                  lithium::sequencer::task::ExposureType::BIAS,
                  lithium::sequencer::task::ExposureType::FLAT,
                  lithium::sequencer::task::ExposureType::SNAPSHOT};
    static constexpr std::array<std::string_view, 5> names = {
        "LIGHT", "DARK", "BIAS", "FLAT", "SNAPSHOT"};
};

template <Enum enumeration>
struct std::formatter<enumeration> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename format_context>
    auto format(enumeration const& e,
                format_context& ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{}", enum_name(e));
    }
};

#define MOCK_CAMERA

namespace lithium::sequencer::task {

#ifdef MOCK_CAMERA
class MockCamera {
public:
    MockCamera() = default;

    bool getExposureStatus() const { return exposureStatus_; }
    void setGain(int g) { gain_ = g; }
    int getGain() const { return gain_; }
    void setOffset(int o) { offset_ = o; }
    int getOffset() const { return offset_; }
    void setBinning(int bx, int by) {
        binningX_ = bx;
        binningY_ = by;
    }
    std::tuple<int, int> getBinning() const { return {binningX_, binningY_}; }
    void startExposure(double t) {
        exposureStatus_ = true;
        exposureTime_ = t;
    }
    void saveExposureResult() { exposureStatus_ = false; }
    bool setFrame(int x, int y, int width, int height) {
        frameX_ = x;
        frameY_ = y;
        frameWidth_ = width;
        frameHeight_ = height;
        return true;
    }
    std::tuple<int, int> getFrame() const {
        return {frameWidth_, frameHeight_};
    }

private:
    bool exposureStatus_{};
    double exposureTime_{};
    int gain_{};
    int offset_{};
    int binningX_{};
    int binningY_{};
    int frameX_{};
    int frameY_{};
    int frameWidth_{};
    int frameHeight_{};
};
#endif

auto TakeExposureTask::taskName() -> std::string { return "TakeExposure"; }

void TakeExposureTask::execute(const json& params) {
    LOG_F(INFO, "Executing TakeExposure task with params: {}", params.dump(4));

    double time = params.at("exposure").get<double>();
    ExposureType type = params.at("type").get<ExposureType>();
    int binning = params.at("binning").get<int>();
    int gain = params.at("gain").get<int>();
    int offset = params.at("offset").get<int>();

    LOG_F(INFO,
          "Starting {} exposure for {} seconds with binning {} and "
          "gain {} and offset {}",
          type, time, binning, gain, offset);

#ifdef MOCK_CAMERA
    std::shared_ptr<MockCamera> camera = std::make_shared<MockCamera>();
#else
    std::shared_ptr<AtomCamera> camera =
        GetPtr<AtomCamera>(Constants::MAIN_CAMERA).value();
#endif

    if (!camera) {
        LOG_F(ERROR, "Main camera not set");
        THROW_RUNTIME_ERROR("Main camera not set");
    }
    if (camera->getExposureStatus()) {
        LOG_F(ERROR, "Main camera is busy");
        THROW_RUNTIME_ERROR("Main camera is busy");
    }
    std::shared_ptr<ConfigManager> configManager =
        GetPtr<ConfigManager>(Constants::CONFIG_MANAGER).value();
    configManager->setValue("/lithium/device/camera/is_exposure", true);
    LOG_F(INFO, "Camera exposure status set to true");

    if (camera->getGain() != gain) {
        LOG_F(INFO, "Setting camera gain to {}", gain);
        camera->setGain(gain);
    }
    if (camera->getOffset() != offset) {
        LOG_F(INFO, "Setting camera offset to {}", offset);
        camera->setOffset(offset);
    }
#ifdef MOCK_CAMERA
    if (camera->getBinning() != std::tuple{binning, binning}) {
        LOG_F(INFO, "Setting camera binning to {}x{}", binning, binning);
        camera->setBinning(binning, binning);
    }
#else
    if (camera->getBinning().value() !=
        std::tuple{binning, binning, binning, binning}) {
        LOG_F(INFO, "Setting camera binning to {}x{}", binning, binning);
        camera->setBinning(binning, binning);
    }
#endif
    // Start exposure
    LOG_F(INFO, "Starting camera exposure for {} seconds", time);
    camera->startExposure(time);

    // Wait for exposure to complete
    atom::async::Timer timer;
    auto exposureFuture = timer.setTimeout(
        [&]() {
            if (camera->getExposureStatus()) {
                LOG_F(ERROR, "Exposure timeout");
                THROW_RUNTIME_ERROR("Exposure timeout");
            }
        },
        time + 1);

    exposureFuture.onComplete([&]() {
        configManager->setValue("/lithium/device/camera/is_exposure", false);
        LOG_F(INFO, "Exposure completed");
    });

    exposureFuture.then([&]() {
        LOG_F(INFO, "Saving exposure result");
        camera->saveExposureResult();
    });

    exposureFuture.get();
    LOG_F(INFO, "Exposure completed");
}

auto TakeManyExposureTask::taskName() -> std::string {
    return "TakeManyExposure";
}

void TakeManyExposureTask::execute(const json& params) {
    LOG_F(INFO, "Executing TakeManyExposure task with params: {}",
          params.dump(4));

    int count = params.at("count").get<int>();
    double time = params.at("exposure").get<double>();
    ExposureType type = params.at("type").get<ExposureType>();
    int binning = params.at("binning").get<int>();
    int gain = params.at("gain").get<int>();
    int offset = params.at("offset").get<int>();

    LOG_F(INFO,
          "Starting {} exposure for {} seconds with binning {} and "
          "gain {} and offset {}",
          static_cast<int>(type), time, binning, gain, offset);

    while (count-- > 0) {
        LOG_F(INFO, "Taking exposure {} of {}", count, count);
        TakeExposureTask::execute(params);
        LOG_F(INFO, "Exposure {} completed", count);
    }
}

auto SubframeExposureTask::taskName() -> std::string {
    return "SubframeExposure";
}

void SubframeExposureTask::execute(const json& params) {
    LOG_F(INFO, "Executing SubframeExposure task with params: {}",
          params.dump(4));

    double time = params.at("exposure").get<double>();
    ExposureType type = params.at("type").get<ExposureType>();
    int binning = params.at("binning").get<int>();
    int gain = params.at("gain").get<int>();
    int offset = params.at("offset").get<int>();
    int x = params.at("x").get<int>();
    int y = params.at("y").get<int>();
    int width = params.at("width").get<int>();
    int height = params.at("height").get<int>();

    LOG_F(INFO,
          "Starting {} exposure for {} seconds with binning {} and "
          "gain {} and offset {} at position ({},{}) with size {}x{}",
          static_cast<int>(type), time, binning, gain, offset, x, y, width,
          height);

#ifdef MOCK_CAMERA
    std::shared_ptr<MockCamera> camera = std::make_shared<MockCamera>();
#else
    std::shared_ptr<AtomCamera> camera =
        GetPtr<AtomCamera>(Constants::MAIN_CAMERA).value();
#endif

    if (!camera) {
        LOG_F(ERROR, "Main camera not set");
        THROW_RUNTIME_ERROR("Main camera not set");
    }
    if (camera->getExposureStatus()) {
        LOG_F(ERROR, "Main camera is busy");
        THROW_RUNTIME_ERROR("Main camera is busy");
    }

#ifdef MOCK_CAMERA
    auto [frameX, frameY] = camera->getFrame();
#else
    auto [frameX, frameY] = camera->getFrame().value();
#endif

    class ROI {
    public:
        ROI(int x, int y, int canvasWidth, int canvasHeight)
            : startX_(x),
              startY_(y),
              canvasWidth_(canvasWidth),
              canvasHeight_(canvasHeight) {}
        [[nodiscard]] auto isOutOfBounds(int x, int y, int width,
                                         int height) const -> bool {
            return x < startX_ || y < startY_ ||
                   x + width > startX_ + canvasWidth_ ||
                   y + height > startY_ + canvasHeight_;
        }

    private:
        int startX_;        // ROI starting x coordinate
        int startY_;        // ROI starting y coordinate
        int canvasWidth_;   // Canvas width
        int canvasHeight_;  // Canvas height
    };
    auto roi = ROI(0, 0, frameX, frameY);
    if (roi.isOutOfBounds(x, y, width, height)) {
        LOG_F(ERROR, "The area is out of bounds!");
        THROW_RUNTIME_ERROR("The area is out of bounds!");
    }

    std::shared_ptr<ConfigManager> configManager =
        GetPtr<ConfigManager>(Constants::CONFIG_MANAGER).value();
    configManager->setValue("/lithium/device/camera/x", x);
    configManager->setValue("/lithium/device/camera/y", y);
    configManager->setValue("/lithium/device/camera/width", width);
    configManager->setValue("/lithium/device/camera/height", height);
    LOG_F(INFO, "Camera frame set to x: {}, y: {}, width: {}, height: {}", x, y,
          width, height);

    if (!camera->setFrame(x, y, width, height)) {
        LOG_F(ERROR, "Failed to set camera frame");
        THROW_RUNTIME_ERROR("Failed to set camera frame");
    }

    TakeExposureTask::execute(params);
    LOG_F(INFO, "Exposure completed");
}

auto SmartExposureTask::taskName() -> std::string { return "SmartExposure"; }

void SmartExposureTask::execute(const json& params) {
    LOG_F(INFO, "Executing SmartExposure task with params: {}", params.dump(4));

    int count = params.at("count").get<int>();
    double time = params.at("exposure").get<double>();
    ExposureType type = params.at("type").get<ExposureType>();
    int binning = params.at("binning").get<int>();
    int gain = params.at("gain").get<int>();
    int offset = params.at("offset").get<int>();
    int filter = params.at("filter").get<int>();
    int ditherPerImage = params.at("ditherPerImage").get<int>();

    if (ditherPerImage > count) {
        LOG_F(ERROR,
              "Dithering interval cannot be greater than the total number of "
              "exposures");
        THROW_RUNTIME_ERROR(
            "Dithering interval cannot be greater than "
            "the total number of exposures");
    }

    LOG_F(INFO,
          "Starting {} exposure for {} seconds with binning {} and "
          "gain {} and offset {} with filter {} and dithering {}",
          static_cast<int>(type), time, binning, gain, offset, filter,
          ditherPerImage);

    while (count-- > 0) {
        LOG_F(INFO, "Taking exposure {} of {}", count, count);
        if (count % ditherPerImage == 0) {
            LOG_F(INFO, "Dithering telescope");
        }
        TakeExposureTask::execute(params);
    }
}

}  // namespace lithium::sequencer::task