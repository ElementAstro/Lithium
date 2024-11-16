#include "ss.h"

#include <libstellarsolver/structuredefinitions.h>
#include <pybind11/stl.h>

#include <utility>

#include "atom/log/loguru.hpp"

// 构造函数
SS::SS(QObject* parent) : QObject(parent), app(nullptr), solver(nullptr) {}

SS::SS(const FITSImage::Statistic& stat, py::buffer buffer, py::object callback, QObject* parent)
    : QObject(parent), callback_(std::move(callback)) {
    LOG_SCOPE_FUNCTION(INFO);

    py::buffer_info info = buffer.request();
    if (info.format != py::format_descriptor<uint8_t>::format()) {
        LOG_F(ERROR, "缓冲区必须是 uint8_t 类型");
        throw std::runtime_error("缓冲区必须是 uint8_t 类型");
    }

    bufferData_.resize(info.size);
    memcpy(bufferData_.data(), info.ptr, info.size);

    int argc = 0;
    char** argv = nullptr;
    app = new QCoreApplication(argc, argv);

    solver = new StellarSolver(stat, bufferData_.data(), this);

    solver->setLogLevel(SSolver::LOG_ALL);

    connect(solver, &StellarSolver::logOutput, this, &SS::onLogOutput);
    connect(solver, &StellarSolver::finished, this, &SS::onFinished);

    LOG_F(INFO, "SS 对象创建成功");
}

SS::~SS() {
    LOG_SCOPE_FUNCTION(INFO);
    delete solver;
    delete app;
    LOG_F(INFO, "SS 对象已销毁");
}

bool SS::loadNewImageBuffer(const FITSImage::Statistic& stats, py::buffer buffer) {
    LOG_SCOPE_FUNCTION(INFO);
    py::buffer_info info = buffer.request();
    if (info.format != py::format_descriptor<uint8_t>::format()) {
        LOG_F(ERROR, "缓冲区必须是 uint8_t 类型");
        return false;
    }

    bufferData_.resize(info.size);
    memcpy(bufferData_.data(), info.ptr, info.size);

    bool result = solver->loadNewImageBuffer(stats, bufferData_.data());
    LOG_F(INFO, "加载新图像缓冲区：{}", result);
    return result;
}

bool SS::extract(bool calculateHFR, QRect frame) {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->extract(calculateHFR, frame);
    LOG_F(INFO, "提取结果：{}", result);
    return result;
}

bool SS::solve() {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->solve();
    LOG_F(INFO, "求解结果：{}", result);
    return result;
}

void SS::start() {
    LOG_SCOPE_FUNCTION(INFO);
    solver->start();
    LOG_F(INFO, "求解器已启动");
}

void SS::abort() {
    LOG_SCOPE_FUNCTION(INFO);
    solver->abort();
    LOG_F(INFO, "求解器已中止");
}

void SS::abortAndWait() {
    LOG_SCOPE_FUNCTION(INFO);
    solver->abortAndWait();
    LOG_F(INFO, "求解器已中止并等待");
}

void SS::setParameterProfile(SSolver::Parameters::ParametersProfile profile) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setParameterProfile(profile);
    LOG_F(INFO, "设置参数配置文件为 {}", profile);
}

void SS::setSearchScale(double fovLow, double fovHigh, const QString& scaleUnits) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchScale(fovLow, fovHigh, scaleUnits);
    LOG_F(INFO, "设置搜索尺度为 {} - {} {}", fovLow, fovHigh, scaleUnits.toStdString());
}

void SS::setSearchScale(double fovLow, double fovHigh, SSolver::ScaleUnits units) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchScale(fovLow, fovHigh, units);
    LOG_F(INFO, "设置搜索尺度为 {} - {} 单位 {}", fovLow, fovHigh, units);
}

void SS::setSearchPositionRaDec(double ra, double dec) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchPositionRaDec(ra, dec);
    LOG_F(INFO, "设置搜索位置 RA: {}, Dec: {}", ra, dec);
}

void SS::setSearchPositionInDegrees(double ra, double dec) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchPositionInDegrees(ra, dec);
    LOG_F(INFO, "设置搜索位置（度） RA: {}, Dec: {}", ra, dec);
}

void SS::setUseSubframe(QRect frame) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setUseSubframe(frame);
    LOG_F(INFO, "设置子帧：x={}, y={}, width={}, height={}", frame.x(), frame.y(), frame.width(), frame.height());
}

bool SS::isRunning() const {
    LOG_SCOPE_FUNCTION(INFO);
    bool running = solver->isRunning();
    LOG_F(INFO, "求解器正在运行：{}", running);
    return running;
}

QString SS::raString(double ra) {
    return StellarSolver::raString(ra);
}

QString SS::decString(double dec) {
    return StellarSolver::decString(dec);
}

bool SS::pixelToWCS(const QPointF& pixelPoint, FITSImage::wcs_point& skyPoint) {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->pixelToWCS(pixelPoint, skyPoint);
    LOG_F(INFO, "像素坐标转换为 WCS：{}", result);
    return result;
}

bool SS::wcsToPixel(const FITSImage::wcs_point& skyPoint, QPointF& pixelPoint) {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->wcsToPixel(skyPoint, pixelPoint);
    LOG_F(INFO, "WCS 转换为像素坐标：{}", result);
    return result;
}

void SS::onLogOutput(const QString& text) {
    LOG_SCOPE_FUNCTION(INFO);
    py::gil_scoped_acquire acquire;
    callback_(text.toStdString());
    LOG_F(INFO, "日志输出: {}", text.toStdString());
}

void SS::onFinished() {
    LOG_SCOPE_FUNCTION(INFO);
    py::gil_scoped_acquire acquire;
    callback_("求解器已完成");
    LOG_F(INFO, "求解器已完成");
}

// 创建星体对象的辅助函数
py::dict SS::createObjectFromStar(const FITSImage::Star& star) {
    LOG_SCOPE_FUNCTION(INFO);
    py::dict obj;
    obj["x"] = star.x;
    obj["y"] = star.y;
    obj["hfr"] = star.HFR;
    obj["flux"] = star.flux;
    obj["ra"] = star.ra;
    obj["dec"] = star.dec;
    LOG_F(INFO, "创建星体对象: x={}, y={}, hfr={}, flux={}, ra={}, dec={}",
          star.x, star.y, star.HFR, star.flux, star.ra, star.dec);
    return obj;
}
