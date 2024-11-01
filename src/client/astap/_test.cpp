#include <fitsio.h>
#include "astap.hpp"

#include <functional>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdio>

#include <gtest/gtest.h>

// 定义一个辅助函数，用于创建用于测试的模拟 FITS 文件
void createMockFitsFile(const std::string& filename) {
    fitsfile* fptr;  // 指向FITS文件的指针
    int status = 0;  // CFITSIO状态值
    long fpixel = 1, nelements;
    long naxis = 2;
    long naxes[2] = {100, 100};  // 图像的尺寸

    // 创建一个新的FITS文件（如果已存在，则覆盖）
    if (fits_create_file(&fptr, ("!" + filename).c_str(), &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }

    // 创建一个简单的16位整数图像 (BITPIX = 16)
    if (fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }

    // 初始化图像数据 (100x100 的像素值设为 100)
    std::vector<short> pixel_value(10000, 100);
    nelements = naxes[0] * naxes[1];  // 图像总的像素数

    // 写入图像数据到FITS文件
    if (fits_write_img(fptr, TSHORT, fpixel, nelements, pixel_value.data(), &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }

    // 关闭FITS文件
    if (fits_close_file(fptr, &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }
}

// 定义一个辅助函数，用于创建用于测试的多个模拟 FITS 文件
void createMockFitsFiles(const std::vector<std::string>& filenames) {
    for (const auto& filename : filenames) {
        createMockFitsFile(filename);
    }
}

class AstapSolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        solver = std::make_unique<AstapSolver>("test_solver");
        createMockFitsFile(testFitsFile);
        solver->initialize();
        solver->scanSolver();
        solver->connect(solverPath, 10, 1);
    }

    void TearDown() override {
        std::remove(testFitsFile.c_str());
        for (const auto& file : tempFiles) {
            std::remove(file.c_str());
        }
        solver->disconnect(true, 10, 1);
    }

    std::unique_ptr<AstapSolver> solver;
    const std::string testFitsFile = "test_image.fits";
    const std::string solverPath = "/usr/bin/astap";  // 请根据实际情况修改
    std::vector<std::string> tempFiles;
};

TEST_F(AstapSolverTest, Initialization) {
    ASSERT_TRUE(solver != nullptr);
    ASSERT_TRUE(solver->isConnected());
}

TEST_F(AstapSolverTest, ConnectWithInvalidPath) {
    solver->disconnect(true, 10, 1);
    ASSERT_FALSE(solver->connect("", 10, 1));
    ASSERT_FALSE(solver->connect("invalid/path", 10, 1));
}

TEST_F(AstapSolverTest, ConnectWithValidPath) {
    solver->disconnect(true, 10, 1);
    ASSERT_TRUE(solver->connect(solverPath, 10, 1));
}

TEST_F(AstapSolverTest, Disconnect) {
    ASSERT_TRUE(solver->isConnected());
    solver->disconnect(true, 10, 1);
    ASSERT_FALSE(solver->isConnected());
}

TEST_F(AstapSolverTest, ScanSolver) {
    ASSERT_TRUE(solver->scanSolver());
}

TEST_F(AstapSolverTest, SolveImageSuccess) {
    ASSERT_TRUE(
        solver->solveImage(testFitsFile, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
                           {}, {}, {}, false, false, false, 60, 0));
}

TEST_F(AstapSolverTest, SolveImageFailure) {
    ASSERT_FALSE(solver->solveImage("non_existent_file.fits", {}, {}, {}, {}, {}, {}, {},
                                    {}, {}, {}, {}, {}, {}, {}, false, false, false, 60, 0));
}

TEST_F(AstapSolverTest, ReadSolveResult) {
    ASSERT_TRUE(
        solver->solveImage(testFitsFile, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
                           {}, {}, {}, false, false, false, 60, 0));
    auto result = solver->getSolveResult(testFitsFile);
    ASSERT_FALSE(result.ra.empty());
    ASSERT_FALSE(result.dec.empty());
    ASSERT_FALSE(result.rotation.empty());
    ASSERT_GT(result.fovAvg, 0.0);
}

TEST_F(AstapSolverTest, AnnotateImage) {
    ASSERT_TRUE(solver->annotateImage(testFitsFile));
    std::string annotatedImage = "test_image_annotated.jpg";
    tempFiles.push_back(annotatedImage);
    ASSERT_TRUE(std::filesystem::exists(annotatedImage));
}

TEST_F(AstapSolverTest, AnalyseImage) {
    ASSERT_TRUE(solver->analyseImage(testFitsFile, 50.0));
    // 可以检查输出日志或生成的文件来验证结果
}

TEST_F(AstapSolverTest, ConvertToFits) {
    std::string sourceImage = "test_image.jpg";
    // 创建一个模拟的JPEG文件
    std::ofstream ofs(sourceImage);
    ofs << "This is a mock JPEG file content.";
    ofs.close();
    tempFiles.push_back(sourceImage);

    ASSERT_TRUE(solver->convertToFits(sourceImage, 2));
    std::string convertedFits = "test_image.fit";
    tempFiles.push_back(convertedFits);
    ASSERT_TRUE(std::filesystem::exists(convertedFits));
}

TEST_F(AstapSolverTest, DebugSolve) {
    // 注意：此测试将打开GUI界面，可能需要手动关闭
    // ASSERT_TRUE(solver->debugSolve(testFitsFile));
}

TEST_F(AstapSolverTest, MeasureSkyBackground) {
    ASSERT_TRUE(solver->measureSkyBackground(testFitsFile, 100.0));
    // 可以检查输出日志或生成的文件来验证结果
}

TEST_F(AstapSolverTest, FindBestFocus) {
    std::vector<std::string> focusFiles = {
        "focus1.fits", "focus2.fits", "focus3.fits", "focus4.fits"};
    createMockFitsFiles(focusFiles);
    tempFiles.insert(tempFiles.end(), focusFiles.begin(), focusFiles.end());

    ASSERT_TRUE(solver->findBestFocus(focusFiles));
    // 可以检查输出日志或返回值来验证结果
}

TEST_F(AstapSolverTest, LiveStack) {
    ASSERT_TRUE(solver->liveStack("."));
    // 可以检查ASTAP是否启动了实时堆叠界面
}

TEST_F(AstapSolverTest, GetSolveResultWithoutSolve) {
    auto result = solver->getSolveResult(testFitsFile);
    ASSERT_TRUE(result.error.empty());
    // 未解决的情况下，结果应为空或默认值
}
