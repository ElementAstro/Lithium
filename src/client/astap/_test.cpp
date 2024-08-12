#include "astap.hpp"
#include <fitsio.h>

#include <string>

#include <gtest/gtest.h>

// Define a helper function to create a mock FITS file for testing
void createMockFitsFile(const std::string& filename) {
    fitsfile *fptr;
    int status = 0;
    fits_create_file(&fptr, ("!" + filename).c_str(), &status);
    fits_create_img(fptr, BYTE_IMG, 0, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "CRVAL1", (void *)&(const double){10.0}, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "CRVAL2", (void *)&(const double){20.0}, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "CDELT1", (void *)&(const double){0.5}, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "CDELT2", (void *)&(const double){0.5}, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "CROTA1", (void *)&(const double){30.0}, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "XPIXSZ", (void *)&(const double){4.8}, nullptr, &status);
    fits_update_key(fptr, TDOUBLE, "YPIXSZ", (void *)&(const double){4.8}, nullptr, &status);
    fits_close_file(fptr, &status);
}

class AstapSolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        solver = std::make_unique<AstapSolver>("test_solver");
        createMockFitsFile(testFitsFile);
    }

    void TearDown() override {
        std::remove(testFitsFile.c_str());
    }

    std::unique_ptr<AstapSolver> solver;
    const std::string testFitsFile = "test_image.fits";
};

TEST_F(AstapSolverTest, Initialization) {
    ASSERT_TRUE(solver != nullptr);
}

TEST_F(AstapSolverTest, ConnectWithInvalidPath) {
    ASSERT_FALSE(solver->connect(""));
    ASSERT_FALSE(solver->connect("invalid/path"));
}

TEST_F(AstapSolverTest, ConnectWithValidPath) {
    // Mock a valid path to the solver
    ASSERT_TRUE(solver->connect("/usr/local/bin/astap-cli"));
}

TEST_F(AstapSolverTest, Disconnect) {
    solver->connect("/usr/local/bin/astap-cli");
    ASSERT_TRUE(solver->isConnected());
    solver->disconnect();
    ASSERT_FALSE(solver->isConnected());
}

TEST_F(AstapSolverTest, ScanSolver) {
    ASSERT_TRUE(solver->scanSolver());
}

TEST_F(AstapSolverTest, SolveImageSuccess) {
    solver->connect("/usr/local/bin/astap-cli");
    ASSERT_TRUE(solver->solveImage(testFitsFile, "10.0", "20.0", 0.5, false, 10, 0));
}

TEST_F(AstapSolverTest, SolveImageFailure) {
    solver->connect("/usr/local/bin/astap-cli");
    ASSERT_FALSE(solver->solveImage("non_existent_file.fits", "10.0", "20.0", 0.5, false, 10, 0));
}

TEST_F(AstapSolverTest, ReadSolveResult) {
    solver->connect("/usr/local/bin/astap-cli");
    solver->solveImage(testFitsFile, "10.0", "20.0", 0.5, false, 10, 0);
    auto result = solver->getSolveResult(testFitsFile);
    ASSERT_EQ(result.ra, "10.0");
    ASSERT_EQ(result.dec, "20.0");
    ASSERT_EQ(result.rotation, "30.0");
    ASSERT_NEAR(result.fovX, 1980.0, 1.0);
    ASSERT_NEAR(result.fovY, 1980.0, 1.0);
    ASSERT_NEAR(result.fovAvg, 1980.0, 1.0);
}
