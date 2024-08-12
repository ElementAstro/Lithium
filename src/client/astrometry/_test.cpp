#include "astrometry.hpp"
#include <gtest/gtest.h>
#include <string>
#include <optional>
#include <fstream>

// Define a helper function to create a mock output for testing
std::string createMockAstrometryOutput() {
    return R"(
Field center: (RA, Dec) = (10.6846, +41.2692)
Field size: 1.0 x 1.0 degrees
Field rotation angle: up is 180.0 degrees
)";
}

class AstrometrySolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        solver = std::make_unique<AstrometrySolver>("test_solver");
    }

    void TearDown() override {
        // Cleanup if necessary
    }

    std::unique_ptr<AstrometrySolver> solver;
};

TEST_F(AstrometrySolverTest, Initialization) {
    ASSERT_TRUE(solver != nullptr);
}

TEST_F(AstrometrySolverTest, ConnectWithInvalidPath) {
    ASSERT_FALSE(solver->connect(""));
    ASSERT_FALSE(solver->connect("invalid/path"));
}

TEST_F(AstrometrySolverTest, ConnectWithValidPath) {
    // Mock a valid path to the solver
    ASSERT_TRUE(solver->connect("/usr/local/bin/astrometry-solver"));
}

TEST_F(AstrometrySolverTest, Disconnect) {
    solver->connect("/usr/local/bin/astrometry-solver");
    ASSERT_TRUE(solver->isConnected());
    solver->disconnect();
    ASSERT_FALSE(solver->isConnected());
}

TEST_F(AstrometrySolverTest, Reconnect) {
    solver->connect("/usr/local/bin/astrometry-solver");
    ASSERT_TRUE(solver->isConnected());
    ASSERT_TRUE(solver->reconnect());
}

TEST_F(AstrometrySolverTest, SolveImageSuccess) {
    solver->connect("/usr/local/bin/astrometry-solver");

    // Mock successful command execution
    std::string mockOutput = createMockAstrometryOutput();
    std::ofstream mockFile("mock_output.txt");
    mockFile << mockOutput;
    mockFile.close();

    // Redirect command output to the mock file
    ASSERT_TRUE(solver->solveImage("mock_image.fits", "10.0", "20.0", 1.0, 2, 4, true, true, 10, 0));

    auto result = solver->getSolveResult("mock_output.txt");
    ASSERT_EQ(result.ra, "10.6846");
    ASSERT_EQ(result.dec, "+41.2692");
    ASSERT_NEAR(result.fovX, 1.0, 0.1);
    ASSERT_NEAR(result.fovY, 1.0, 0.1);
    ASSERT_NEAR(result.fovAvg, 1.0, 0.1);
    ASSERT_NEAR(result.rotation, 180.0, 0.1);
}

TEST_F(AstrometrySolverTest, SolveImageFailure) {
    solver->connect("/usr/local/bin/astrometry-solver");
    ASSERT_FALSE(solver->solveImage("non_existent_file.fits", "10.0", "20.0", 1.0, 2, 4, true, true, 10, 0));
}

TEST_F(AstrometrySolverTest, ReadSolveResult) {
    std::string mockOutput = createMockAstrometryOutput();
    auto result = solver->readSolveResult(mockOutput);
    ASSERT_EQ(result.ra, "10.6846");
    ASSERT_EQ(result.dec, "+41.2692");
    ASSERT_NEAR(result.fovX, 1.0, 0.1);
    ASSERT_NEAR(result.fovY, 1.0, 0.1);
    ASSERT_NEAR(result.fovAvg, 1.0, 0.1);
    ASSERT_NEAR(result.rotation, 180.0, 0.1);
}
