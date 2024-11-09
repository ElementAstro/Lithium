#ifndef ATOM_ALGORITHM_TEST_ANNEALING_HPP
#define ATOM_ALGORITHM_TEST_ANNEALING_HPP

#include <gtest/gtest.h>
#include "atom/algorithm/annealing.hpp"

// Test fixture for TSP tests
class TSPTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize cities for testing
        cities_ = {
            {0.0, 0.0},
            {1.0, 0.0},
            {1.0, 1.0},
            {0.0, 1.0}
        };
        tsp_ = std::make_unique<TSP>(cities_);
    }

    std::vector<std::pair<double, double>> cities_;
    std::unique_ptr<TSP> tsp_;
};

// Test case for energy calculation with a valid solution
TEST_F(TSPTest, EnergyCalculationValidSolution) {
    std::vector<int> solution = {0, 1, 2, 3};
    double energy = tsp_->energy(solution);
    double expected_energy = 4.0; // Perimeter of the square
    EXPECT_DOUBLE_EQ(energy, expected_energy);
}

// Test case for energy calculation with a different valid solution
TEST_F(TSPTest, EnergyCalculationDifferentSolution) {
    std::vector<int> solution = {0, 2, 1, 3};
    double energy = tsp_->energy(solution);
    double expected_energy = 4.82842712474619; // Perimeter with diagonal
    EXPECT_DOUBLE_EQ(energy, expected_energy);
}

// Test case for energy calculation with an invalid solution (duplicate cities)
TEST_F(TSPTest, EnergyCalculationInvalidSolution) {
    std::vector<int> solution = {0, 1, 1, 3};
    double energy = tsp_->energy(solution);
    // The energy calculation should still work, but the result may not be meaningful
    EXPECT_TRUE(std::isfinite(energy));
}

// Test case for energy calculation with an empty solution
TEST_F(TSPTest, EnergyCalculationEmptySolution) {
    std::vector<int> solution = {};
    double energy = tsp_->energy(solution);
    double expected_energy = 0.0;
    EXPECT_DOUBLE_EQ(energy, expected_energy);
}

// Test case for energy calculation with a single city
TEST_F(TSPTest, EnergyCalculationSingleCity) {
    std::vector<int> solution = {0};
    double energy = tsp_->energy(solution);
    double expected_energy = 0.0;
    EXPECT_DOUBLE_EQ(energy, expected_energy);
}

// Test case for energy calculation with two cities
TEST_F(TSPTest, EnergyCalculationTwoCities) {
    std::vector<int> solution = {0, 1};
    double energy = tsp_->energy(solution);
    double expected_energy = 2.0; // Distance from (0,0) to (1,0) and back
    EXPECT_DOUBLE_EQ(energy, expected_energy);
}

#endif  // ATOM_ALGORITHM_TEST_ANNEALING_HPP
