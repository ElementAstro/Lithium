#include "ss.h"

#include <fitsio.h>
#include <libstellarsolver/structuredefinitions.h>
#include <pybind11/stl.h>

#include <utility>

#include "atom/log/loguru.hpp"

// Constructor
SS::SS(QObject* parent) : QObject(parent), app(nullptr), solver(nullptr) {}

SS::SS(const FITSImage::Statistic& stat, py::buffer buffer, py::object callback,
       QObject* parent)
    : QObject(parent), callback_(std::move(callback)) {
    LOG_SCOPE_FUNCTION(INFO);

    py::buffer_info info = buffer.request();
    if (info.format != py::format_descriptor<uint8_t>::format()) {
        LOG_F(ERROR, "Buffer must be of type uint8_t");
        throw std::runtime_error("Buffer must be of type uint8_t");
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

    LOG_F(INFO, "SS object created successfully");
}

SS::~SS() {
    LOG_SCOPE_FUNCTION(INFO);
    delete solver;
    delete app;
    LOG_F(INFO, "SS object destroyed");
}

bool SS::loadNewImageBuffer(const FITSImage::Statistic& stats,
                            py::buffer buffer) {
    LOG_SCOPE_FUNCTION(INFO);
    py::buffer_info info = buffer.request();
    if (info.format != py::format_descriptor<uint8_t>::format()) {
        LOG_F(ERROR, "Buffer must be of type uint8_t");
        return false;
    }

    bufferData_.resize(info.size);
    memcpy(bufferData_.data(), info.ptr, info.size);

    bool result = solver->loadNewImageBuffer(stats, bufferData_.data());
    LOG_F(INFO, "Loaded new image buffer: {}", result);
    return result;
}

bool SS::extract(bool calculateHFR, QRect frame) {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->extract(calculateHFR, frame);
    LOG_F(INFO, "Extraction result: {}", result);
    return result;
}

bool SS::solve() {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->solve();
    LOG_F(INFO, "Solve result: {}", result);
    return result;
}

void SS::start() {
    LOG_SCOPE_FUNCTION(INFO);
    solver->start();
    LOG_F(INFO, "Solver started");
}

void SS::abort() {
    LOG_SCOPE_FUNCTION(INFO);
    solver->abort();
    LOG_F(INFO, "Solver aborted");
}

void SS::abortAndWait() {
    LOG_SCOPE_FUNCTION(INFO);
    solver->abortAndWait();
    LOG_F(INFO, "Solver aborted and waiting");
}

void SS::setParameterProfile(SSolver::Parameters::ParametersProfile profile) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setParameterProfile(profile);
    LOG_F(INFO, "Set parameter profile to {}", profile);
}

void SS::setSearchScale(double fovLow, double fovHigh,
                        const QString& scaleUnits) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchScale(fovLow, fovHigh, scaleUnits);
    LOG_F(INFO, "Set search scale to {} - {} {}", fovLow, fovHigh,
          scaleUnits.toStdString());
}

void SS::setSearchScale(double fovLow, double fovHigh,
                        SSolver::ScaleUnits units) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchScale(fovLow, fovHigh, units);
    LOG_F(INFO, "Set search scale to {} - {} units {}", fovLow, fovHigh, units);
}

void SS::setSearchPositionRaDec(double ra, double dec) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchPositionRaDec(ra, dec);
    LOG_F(INFO, "Set search position RA: {}, Dec: {}", ra, dec);
}

void SS::setSearchPositionInDegrees(double ra, double dec) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setSearchPositionInDegrees(ra, dec);
    LOG_F(INFO, "Set search position (degrees) RA: {}, Dec: {}", ra, dec);
}

void SS::setUseSubframe(QRect frame) {
    LOG_SCOPE_FUNCTION(INFO);
    solver->setUseSubframe(frame);
    LOG_F(INFO, "Set subframe: x={}, y={}, width={}, height={}", frame.x(),
          frame.y(), frame.width(), frame.height());
}

bool SS::isRunning() const {
    LOG_SCOPE_FUNCTION(INFO);
    bool running = solver->isRunning();
    LOG_F(INFO, "Solver is running: {}", running);
    return running;
}

QString SS::raString(double ra) { return StellarSolver::raString(ra); }

QString SS::decString(double dec) { return StellarSolver::decString(dec); }

bool SS::pixelToWCS(const QPointF& pixelPoint, FITSImage::wcs_point& skyPoint) {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->pixelToWCS(pixelPoint, skyPoint);
    LOG_F(INFO, "Pixel to WCS: {}", result);
    return result;
}

bool SS::wcsToPixel(const FITSImage::wcs_point& skyPoint, QPointF& pixelPoint) {
    LOG_SCOPE_FUNCTION(INFO);
    bool result = solver->wcsToPixel(skyPoint, pixelPoint);
    LOG_F(INFO, "WCS to pixel: {}", result);
    return result;
}

void SS::onLogOutput(const QString& text) {
    LOG_SCOPE_FUNCTION(INFO);
    py::gil_scoped_acquire acquire;
    callback_(text.toStdString());
    LOG_F(INFO, "Log output: {}", text.toStdString());
}

void SS::onFinished() {
    LOG_SCOPE_FUNCTION(INFO);
    py::gil_scoped_acquire acquire;
    callback_("Solver finished");
    LOG_F(INFO, "Solver finished");
}

// Helper function to create a star object
py::dict SS::createObjectFromStar(const FITSImage::Star& star) {
    LOG_SCOPE_FUNCTION(INFO);
    py::dict obj;
    obj["x"] = star.x;
    obj["y"] = star.y;
    obj["hfr"] = star.HFR;
    obj["flux"] = star.flux;
    obj["ra"] = star.ra;
    obj["dec"] = star.dec;
    LOG_F(INFO,
          "Created star object: x={}, y={}, hfr={}, flux={}, ra={}, dec={}",
          star.x, star.y, star.HFR, star.flux, star.ra, star.dec);
    return obj;
}

auto loadFits(const QString& fileName) -> LoadFitsResult {
    LOG_SCOPE_FUNCTION(INFO);
    LoadFitsResult result;
    fitsfile* fptr{nullptr};
    int status = 0;
    int anynullptr = 0;
    long naxes[3] = {0, 0, 0};

    // Open FITS file
    if (fits_open_diskfile(&fptr, fileName.toLocal8Bit(), READONLY, &status)) {
        LOG_F(ERROR, "Error opening FITS file: {}", fileName.toStdString());
        return result;
    }

    // Set file size
    result.imageStats.size = QFile(fileName).size();
    LOG_F(INFO, "File size: {} bytes", result.imageStats.size);

    // Move to image HDU
    if (fits_movabs_hdu(fptr, 1, IMAGE_HDU, &status)) {
        LOG_F(ERROR, "Could not locate image HDU");
        fits_close_file(fptr, &status);
        return result;
    }

    // Get image parameters
    int fitsBitPix = 0;
    if (fits_get_img_param(fptr, 3, &fitsBitPix, &(result.imageStats.ndim),
                           naxes, &status)) {
        LOG_F(ERROR, "FITS file error: could not get image parameters");
        fits_close_file(fptr, &status);
        return result;
    }

    // Validate dimensions
    if (result.imageStats.ndim < 2) {
        LOG_F(ERROR, "1D FITS images are not supported");
        fits_close_file(fptr, &status);
        return result;
    }

    // Set data type and bytes per pixel
    switch (fitsBitPix) {
        case BYTE_IMG:
            result.imageStats.dataType = 11;  // SEP_TBYTE
            result.imageStats.bytesPerPixel = sizeof(uint8_t);
            break;
        case SHORT_IMG:
        case USHORT_IMG:
            result.imageStats.dataType = TUSHORT;
            result.imageStats.bytesPerPixel = sizeof(uint16_t);
            break;
        case LONG_IMG:
        case ULONG_IMG:
            result.imageStats.dataType = TULONG;
            result.imageStats.bytesPerPixel = sizeof(uint32_t);
            break;
        case FLOAT_IMG:
            result.imageStats.dataType = TFLOAT;
            result.imageStats.bytesPerPixel = sizeof(float);
            break;
        case LONGLONG_IMG:
            result.imageStats.dataType = TLONGLONG;
            result.imageStats.bytesPerPixel = sizeof(int64_t);
            break;
        case DOUBLE_IMG:
            result.imageStats.dataType = TDOUBLE;
            result.imageStats.bytesPerPixel = sizeof(double);
            break;
        default:
            LOG_F(ERROR, "Unsupported bit depth: {}", fitsBitPix);
            fits_close_file(fptr, &status);
            return result;
    }

    // Set image dimensions
    if (result.imageStats.ndim < 3)
        naxes[2] = 1;
    if (naxes[0] == 0 || naxes[1] == 0) {
        LOG_F(ERROR, "Invalid dimensions: {}x{}", naxes[0], naxes[1]);
        fits_close_file(fptr, &status);
        return result;
    }

    result.imageStats.width = static_cast<uint16_t>(naxes[0]);
    result.imageStats.height = static_cast<uint16_t>(naxes[1]);
    result.imageStats.channels = static_cast<uint8_t>(naxes[2]);
    result.imageStats.samples_per_channel =
        result.imageStats.width * result.imageStats.height;

    // Allocate buffer
    const uint32_t bufferSize =
        result.imageStats.samples_per_channel * result.imageStats.channels *
        static_cast<uint32_t>(result.imageStats.bytesPerPixel);

    try {
        result.imageBuffer = new uint8_t[bufferSize];
    } catch (const std::bad_alloc& e) {
        LOG_F(ERROR, "Memory allocation failed for {} bytes: {}", bufferSize,
              e.what());
        fits_close_file(fptr, &status);
        return result;
    }

    // Read image data
    const long NELEMENTS =
        result.imageStats.samples_per_channel * result.imageStats.channels;
    if (fits_read_img(fptr, static_cast<uint16_t>(result.imageStats.dataType),
                      1, NELEMENTS, nullptr, result.imageBuffer, &anynullptr,
                      &status)) {
        LOG_F(ERROR, "Error reading image data");
        delete[] result.imageBuffer;
        fits_close_file(fptr, &status);
        return result;
    }

    fits_close_file(fptr, &status);
    result.success = true;
    LOG_F(INFO, "Successfully loaded FITS image: {}x{}x{}",
          result.imageStats.width, result.imageStats.height,
          result.imageStats.channels);

    return result;
}

auto SS::findStarsByStellarSolver(bool AllStars,
                                  bool runHFR) -> QList<FITSImage::Star> {
    LoadFitsResult result;

    QList<FITSImage::Star> stars;

    result = loadFits("/dev/shm/ccd_simulator.fits");

    if (!result.success) {
        LOG_F(ERROR, "Error in loading FITS file");
        return stars;
    }

    FITSImage::Statistic imageStats = result.imageStats;
    uint8_t* imageBuffer = result.imageBuffer;
    stars = findStarsByStellarSolver(AllStars, imageStats, imageBuffer, runHFR);
    return stars;
}

auto SS::findStarsByStellarSolver(bool AllStars,
                                  const FITSImage::Statistic& imagestats,
                                  const uint8_t* imageBuffer,
                                  bool runHFR) -> QList<FITSImage::Star> {
    StellarSolver solver(imagestats, imageBuffer);
    // 配置solver参数
    SSolver::Parameters parameters;

    // 设置参数
    // parameters.apertureShape = SSolver::SHAPE_CIRCLE; //
    // 使用圆形的星点检测形状 parameters.kron_fact = 2.5; // 设置Kron因子
    // parameters.subpix = 5;                            // 子像素设置
    // parameters.r_min = 5;                             // 最小星点半径
    // parameters.magzero = 20;                          // 零点星等
    // parameters.minarea = 20;                          // 最小星点面积
    // parameters.deblend_thresh = 32;                   // 去混叠阈值
    // parameters.deblend_contrast = 0.005;              // 去混叠对比度
    // parameters.clean = 1;                             // 清理图像
    // parameters.fwhm = 1;                              // 全宽半高
    // parameters.maxSize = 0;                           // 最大星点大小
    // parameters.minSize = 0;                           // 最小星点大小
    // parameters.maxEllipse = 1.5;                      // 最大椭圆比
    // parameters.initialKeep = 250;                     // 初始保留星点数量
    // parameters.keepNum = 100;                         // 保留星点数量
    // parameters.removeBrightest = 10;                  // 移除最亮星点比例
    // parameters.removeDimmest = 20;                    // 移除最暗星点比例
    // parameters.saturationLimit = 90;                  // 饱和度限制

    parameters.apertureShape = SSolver::SHAPE_CIRCLE;
    parameters.autoDownsample = true;
    parameters.clean = 1;
    parameters.clean_param = 1;
    parameters.convFilterType = SSolver::CONV_GAUSSIAN;
    parameters.deblend_contrast = 0.004999999888241291;
    parameters.deblend_thresh = 32;
    parameters.description = "Default focus star-extraction.";
    parameters.downsample = 1;
    parameters.fwhm = 1;
    parameters.inParallel = true;
    parameters.initialKeep = 250;
    parameters.keepNum = 100;
    parameters.kron_fact = 2.5;
    parameters.listName = "1-Focus-Default";
    parameters.logratio_tokeep = 20.72326583694641;
    parameters.logratio_tosolve = 20.72326583694641;
    parameters.logratio_totune = 13.815510557964274;
    parameters.magzero = 20;
    parameters.maxEllipse = 1.5;
    parameters.maxSize = 10;
    parameters.maxwidth = 180;
    parameters.minSize = 0;
    parameters.minarea = 20;
    parameters.minwidth = 0.1;
    parameters.multiAlgorithm = SSolver::MULTI_AUTO;
    parameters.partition = true;
    parameters.r_min = 5;
    parameters.removeBrightest = 10;
    parameters.removeDimmest = 20;
    parameters.resort = true;
    parameters.saturationLimit = 90;
    parameters.search_parity = 15;
    parameters.solverTimeLimit = 600;
    parameters.subpix = 5;

    solver.setLogLevel(SSolver::LOG_ALL);
    solver.setSSLogLevel(SSolver::LOG_NORMAL);

    solver.setProperty("ExtractorType", SSolver::EXTRACTOR_INTERNAL);
    solver.setProperty("ProcessType", SSolver::EXTRACT);
    solver.setParameterProfile(SSolver::Parameters::DEFAULT);
    // solver.setParameterProfile(SSolver::Parameters::ALL_STARS);

    // 设置参数
    solver.setParameters(parameters);

    if (AllStars) {
        solver.setParameterProfile(SSolver::Parameters::ALL_STARS);
    }

    // 进行星点检测
    bool success = solver.extract(runHFR);
    if (!success) {
        LOG_F(ERROR, "Star extraction failed.");
    }
    LOG_F(INFO, "success extract: {}", success);

    QList<FITSImage::Star> stars;

    stars = solver.getStarList();

    // 输出检测到的星点信息
    LOG_F(INFO, "Detected stars: {}", stars.size());
    for (const auto& star : stars) {
        LOG_F(INFO, "Star: x={}, y={}, HFR={}, flux={}, ra={}, dec={}", star.x,
              star.y, star.HFR, star.flux, star.ra, star.dec);
    }

    return stars;
}