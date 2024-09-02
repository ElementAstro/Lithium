#include <fitsio.h>
#include "astap.hpp"

#include <functional>
#include <string>

#include <gtest/gtest.h>

// Define a helper function to create a mock FITS file for testing
void createMockFitsFile(const std::string& filename) {
    fitsfile* fptr;  // 指向FITS文件的指针
    int status = 0;  // CFITSIO状态值
    long fpixel = 1, naxis = 2, nelements;
    long naxes[2] = {100, 100};  // 图像的尺寸

    // 创建一个新的FITS文件（如果已存在，则覆盖）
    if (fits_create_file(&fptr, "!example.fits", &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }

    // 创建一个简单的8位无符号整型图像 (BITPIX = 8)
    if (fits_create_img(fptr, BYTE_IMG, naxis, naxes, &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }

    // 初始化图像数据 (100x100的像素值设为1)
    unsigned char pixel_value[10000];
    for (int i = 0; i < 10000; i++) {
        pixel_value[i] = 1;
    }
    nelements = naxes[0] * naxes[1];  // 图像总的像素数

    // 写入图像数据到FITS文件
    if (fits_write_img(fptr, TBYTE, fpixel, nelements, pixel_value, &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }

    // 关闭FITS文件
    if (fits_close_file(fptr, &status)) {
        fits_report_error(stderr, status);  // 报告错误
        return;
    }
}

class AstapSolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        solver = std::make_unique<AstapSolver>("test_solver");
        createMockFitsFile(testFitsFile);
    }

    void TearDown() override { std::remove(testFitsFile.c_str()); }

    std::unique_ptr<AstapSolver> solver;
    const std::string testFitsFile = "test_image.fits";
};

TEST_F(AstapSolverTest, Initialization) { ASSERT_TRUE(solver != nullptr); }

TEST_F(AstapSolverTest, ConnectWithInvalidPath) {
    ASSERT_FALSE(solver->connect("", 10, 1));
    ASSERT_FALSE(solver->connect("invalid/path", 10, 1));
}

TEST_F(AstapSolverTest, ConnectWithValidPath) {
    // Mock a valid path to the solver
    ASSERT_TRUE(solver->connect("/usr/local/bin/astap-cli", 10, 1));
}

TEST_F(AstapSolverTest, Disconnect) {
    solver->connect("/usr/local/bin/astap-cli", 10, 1);
    ASSERT_TRUE(solver->isConnected());
    solver->disconnect(true, 10, 1);
    ASSERT_FALSE(solver->isConnected());
}

TEST_F(AstapSolverTest, ScanSolver) { ASSERT_TRUE(solver->scanSolver()); }

TEST_F(AstapSolverTest, SolveImageSuccess) {
    solver->connect("/usr/local/bin/astap-cli", 10, 1);
    ASSERT_TRUE(
        solver->solveImage(testFitsFile, "10.0", "20.0", 0.5, false, 10, 0));
}

TEST_F(AstapSolverTest, SolveImageFailure) {
    solver->connect("/usr/local/bin/astap-cli", 10, 1);
    ASSERT_FALSE(solver->solveImage("non_existent_file.fits", "10.0", "20.0",
                                    0.5, false, 10, 0));
}

TEST_F(AstapSolverTest, ReadSolveResult) {
    solver->connect("/usr/local/bin/astap-cli", 10, 1);
    solver->solveImage(testFitsFile, "10.0", "20.0", 0.5, false, 10, 0);
    auto result = solver->getSolveResult(testFitsFile);
    ASSERT_EQ(result.ra, "10.0");
    ASSERT_EQ(result.dec, "20.0");
    ASSERT_EQ(result.rotation, "30.0");
    ASSERT_NEAR(result.fovX, 1980.0, 1.0);
    ASSERT_NEAR(result.fovY, 1980.0, 1.0);
    ASSERT_NEAR(result.fovAvg, 1980.0, 1.0);
}
