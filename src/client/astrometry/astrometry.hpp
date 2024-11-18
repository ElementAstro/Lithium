// astrometry.hpp
#ifndef LITHIUM_CLIENT_ASTROMETRY_HPP
#define LITHIUM_CLIENT_ASTROMETRY_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

class SolveResult {
public:
    std::string ra;
    std::string dec;
    std::string rotation;
    double fovX;
    double fovY;
    double fovAvg;
    std::string error;
};

struct SolveResults {
    double RA_Degree = 0;
    double DEC_Degree = 0;
    double RA_0 = 0, DEC_0 = 0;
    double RA_1 = 0, DEC_1 = 0;
    double RA_2 = 0, DEC_2 = 0;
    double RA_3 = 0, DEC_3 = 0;
};

struct SolveOptions {
    // 基本选项
    std::optional<std::string> backend_config;
    std::optional<std::string> config;
    bool batch = false;
    bool files_on_stdin = false;
    bool no_plots = false;
    std::optional<double> plot_scale;
    std::optional<std::string> plot_bg;
    bool use_wget = false;
    bool overwrite = false;
    bool continue_run = false;
    bool skip_solved = false;
    bool fits_image = false;
    std::optional<std::string> new_fits;
    std::optional<std::string> kmz;
    std::optional<std::string> scamp;
    std::optional<std::string> scamp_config;
    std::optional<std::string> index_xyls;
    bool just_augment = false;
    std::optional<std::string> axy;
    bool temp_axy = false;
    bool timestamp = false;
    bool no_delete_temp = false;

    // 规模相关选项
    std::optional<double> scale_low;
    std::optional<double> scale_high;
    std::optional<std::string> scale_units;

    // 奇偶性和容差
    std::optional<std::string> parity;
    std::optional<double> code_tolerance;
    std::optional<int> pixel_error;

    // 四边形大小
    std::optional<double> quad_size_min;
    std::optional<double> quad_size_max;

    // 概率相关
    std::optional<double> odds_to_tune_up;
    std::optional<double> odds_to_solve;
    std::optional<double> odds_to_reject;
    std::optional<double> odds_to_stop_looking;

    // 来源提取器
    bool use_source_extractor = false;
    std::optional<std::string> source_extractor_config;
    std::optional<std::string> source_extractor_path;

    // 场中心
    std::optional<std::string> ra;
    std::optional<std::string> dec;
    std::optional<double> radius;

    // 深度和对象
    std::optional<int> depth;
    std::optional<int> objs;

    // CPU限制和排序
    std::optional<int> cpulimit;
    bool resort = false;

    // FITS扩展和图像处理
    std::optional<int> extension;
    bool invert = false;
    std::optional<int> downsample;
    bool no_background_subtraction = false;
    std::optional<float> sigma;
    std::optional<float> nsigma;
    bool no_remove_lines = false;
    std::optional<int> uniformize;
    bool no_verify_uniformize = false;
    bool no_verify_dedup = false;

    // 取消和解决文件
    std::optional<std::string> cancel;
    std::optional<std::string> solved;
    std::optional<std::string> solved_in;
    std::optional<std::string> match;
    std::optional<std::string> rdls;
    std::optional<std::string> sort_rdls;
    std::optional<std::string> tag;
    bool tag_all = false;

    // SCAMP相关
    std::optional<std::string> scamp_ref;
    std::optional<std::string> corr;
    std::optional<std::string> wcs;
    std::optional<std::string> pnm;
    std::optional<std::string> keep_xylist;
    bool dont_augment = false;
    std::optional<std::string> verify;
    std::optional<std::string> verify_ext;
    bool no_verify = false;
    bool guess_scale = false;
    bool crpix_center = false;
    std::optional<int> crpix_x;
    std::optional<int> crpix_y;
    bool no_tweak = false;
    std::optional<int> tweak_order;
    std::optional<std::string> predistort;
    std::optional<double> xscale;
    std::optional<std::string> temp_dir;
};

class AstrometrySolver {
public:
    explicit AstrometrySolver(std::string name);
    ~AstrometrySolver();

    auto connect(std::string_view solverPath) -> bool;

    auto disconnect() -> bool;

    auto reconnect() -> bool;

    auto isConnected() -> bool;

    auto scanSolver() -> std::vector<std::string>;

    auto solveImage(std::string_view image,
                    std::optional<std::string_view> target_ra,
                    std::optional<std::string_view> target_dec,
                    std::optional<double> radius, std::optional<int> downsample,
                    std::optional<int> depth, bool overWrite, bool noPlot,
                    int timeout, int debug,
                    const SolveOptions& options = SolveOptions()) -> bool;

    auto plateSolve(const std::string& filename, int focalLength,
                    double cameraSizeWidth,
                    double cameraSizeHeight) -> SolveResults;

    auto getSolveResult(std::string_view image) -> SolveResult;

    auto readSolveResult(const std::string& output) -> SolveResult;

private:
    std::string name_;
    std::string solverPath_;
    std::string solverVersion_;

    SolveResult solveResult_;

    static bool plateSolveInProgress;
    static bool isSolveImageFinished;

    auto readSolveResult(const std::string& filename, int imageWidth,
                         int imageHeight) -> SolveResults;

    // 辅助方法
    std::string buildCommand(std::string_view image,
                             std::optional<std::string_view> target_ra,
                             std::optional<std::string_view> target_dec,
                             std::optional<double> radius,
                             std::optional<int> downsample,
                             std::optional<int> depth, bool overWrite,
                             bool noPlot, int timeout, int debug,
                             const SolveOptions& options);
};

#endif