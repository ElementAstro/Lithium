#ifndef LITHIUM_CLIENT_ASTAP_HPP
#define LITHIUM_CLIENT_ASTAP_HPP

#include "device/template/solver.hpp"

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

class AstapSolver : public AtomSolver {
public:
    explicit AstapSolver(std::string name);
    ~AstapSolver();

    auto initialize() -> bool override;

    auto destroy() -> bool override;

    auto connect(const std::string& name, int timeout,
                 int maxRetry) -> bool override;

    auto disconnect(bool force, int timeout, int maxRetry) -> bool override;

    auto reconnect(int timeout, int maxRetry) -> bool override;

    auto scan() -> std::vector<std::string> override;

    auto isConnected() -> bool override;

    auto scanSolver() -> bool;

    auto solve(const std::string& imageFilePath,
               const std::optional<Coordinates>& initialCoordinates,
               double fovW, double fovH, int imageWidth,
               int imageHeight) -> PlateSolveResult override;

    auto solveImage(std::string_view image,
                    std::optional<double> radius_search_field = {},
                    std::optional<double> field_height = {},
                    std::optional<double> ra = {},
                    std::optional<double> spd = {},
                    std::optional<int> downsample_factor = {},
                    std::optional<int> max_stars = {},
                    std::optional<double> tolerance = {},
                    std::optional<double> min_star_size = {},
                    std::optional<bool> apply_check = {},
                    std::optional<std::string> database_path = {},
                    std::optional<std::string> database_abbreviation = {},
                    std::optional<std::string> output_file = {},
                    std::optional<bool> add_sip = {},
                    std::optional<std::string> speed_mode = {},
                    bool write_wcs = false, bool update = false,
                    bool log = false, int timeout = 60, int debug = 0) -> bool;

    auto analyseImage(std::string_view image, double snr_minimum,
                      bool extract = false, bool extract2 = false) -> bool;

    auto convertToFits(std::string_view image, int binning) -> bool;

    auto annotateImage(std::string_view image) -> bool;

    auto debugSolve(std::string_view image) -> bool;

    auto measureSkyBackground(std::string_view image, double pedestal) -> bool;

    auto findBestFocus(const std::vector<std::string>& image_files) -> bool;

    auto liveStack(std::string_view path) -> bool;

    auto getSolveResult(std::string_view image) -> SolveResult;

private:
    auto readSolveResult(std::string_view image) -> SolveResult;

    std::string name_;
    std::string solverPath_;
    std::string solverVersion_;
};

#endif
