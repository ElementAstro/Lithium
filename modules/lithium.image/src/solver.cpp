#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <uuid/uuid.h>

#include "solver.hpp"

#include <libstellarsolver/stellarsolver.h>

auto findStarsByStellarSolver(bool AllStars, bool runHFR) -> std::vector<FITSImage::Star> {
    Tools tempTool;

    LoadFitsResult result;

    std::vector<FITSImage::Star> stars;

    result = loadFits("/dev/shm/ccd_simulator.fits");

    if (!result.success) {
        std::cerr << "Error in loading FITS file" << std::endl;
        return stars;
    }

    FITSImage::Statistic imageStats = result.imageStats;
    uint8_t* imageBuffer = result.imageBuffer;
    stars = tempTool.FindStarsByStellarSolver_(AllStars, imageStats, imageBuffer, runHFR);
    return stars;
}

std::vector<FITSImage::Star> FindStarsByStellarSolver_(bool AllStars, const FITSImage::Statistic& imagestats, const uint8_t* imageBuffer, bool runHFR) {
    StellarSolver solver(imagestats, imageBuffer);
    // 配置solver参数
    SSolver::Parameters parameters;

    // 设置参数
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

    solver.setParameters(parameters);

    if (AllStars) {
        solver.setParameterProfile(SSolver::Parameters::ALL_STARS);
    }

    // 进行星点检测
    bool success = solver.extract(runHFR);
    if (!success) {
        std::cerr << "Star extraction failed." << std::endl;
    }
    std::cout << "Success extract: " << success << std::endl;

    std::vector<FITSImage::Star> stars = solver.getStarList();

    // 输出检测到的星点信息
    std::cout << "Detected " << stars.size() << " stars." << std::endl;
    for (const auto& star : stars) {
        std::cout << "Star at (" << star.x << ", " << star.y << ") with HFR: " << star.HFR << std::endl;
    }

    return stars;
}

void StellarSolverLogOutput(const std::string& text) {
    std::cout << "StellarSolver LogOutput: " << text << std::endl;
}
