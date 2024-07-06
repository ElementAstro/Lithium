#include "atom/algorithm/perlin.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

class PerlinNoiseTest : public ::testing::Test {
protected:
    PerlinNoise perlin;
};

TEST_F(PerlinNoiseTest, NoiseRangeTest) {
    for (int i = 0; i < 1000; ++i) {
        double x = static_cast<double>(rand()) / RAND_MAX * 100;
        double y = static_cast<double>(rand()) / RAND_MAX * 100;
        double z = static_cast<double>(rand()) / RAND_MAX * 100;
        double noise = perlin.noise(x, y, z);
        EXPECT_GE(noise, 0.0);
        EXPECT_LE(noise, 1.0);
    }
}

TEST_F(PerlinNoiseTest, ConsistencyTest) {
    double noise1 = perlin.noise(1.0, 2.0, 3.0);
    double noise2 = perlin.noise(1.0, 2.0, 3.0);
    EXPECT_DOUBLE_EQ(noise1, noise2);
}

TEST_F(PerlinNoiseTest, OctaveNoiseRangeTest) {
    for (int i = 0; i < 1000; ++i) {
        double x = static_cast<double>(rand()) / RAND_MAX * 100;
        double y = static_cast<double>(rand()) / RAND_MAX * 100;
        double z = static_cast<double>(rand()) / RAND_MAX * 100;
        double noise = perlin.octaveNoise(x, y, z, 4, 0.5);
        EXPECT_GE(noise, 0.0);
        EXPECT_LE(noise, 1.0);
    }
}

TEST_F(PerlinNoiseTest, OctaveNoiseConsistencyTest) {
    double noise1 = perlin.octaveNoise(1.0, 2.0, 3.0, 4, 0.5);
    double noise2 = perlin.octaveNoise(1.0, 2.0, 3.0, 4, 0.5);
    EXPECT_DOUBLE_EQ(noise1, noise2);
}

TEST_F(PerlinNoiseTest, NoiseMapDimensionsTest) {
    int width = 10, height = 15;
    auto noiseMap = perlin.generateNoiseMap(width, height, 1.0, 4, 0.5, 2.0);
    EXPECT_EQ(noiseMap.size(), height);
    for (const auto& row : noiseMap) {
        EXPECT_EQ(row.size(), width);
    }
}

TEST_F(PerlinNoiseTest, NoiseMapRangeTest) {
    int width = 10, height = 15;
    auto noiseMap = perlin.generateNoiseMap(width, height, 1.0, 4, 0.5, 2.0);
    for (const auto& row : noiseMap) {
        for (double value : row) {
            EXPECT_GE(value, 0.0);
            EXPECT_LE(value, 1.0);
        }
    }
}

TEST_F(PerlinNoiseTest, NoiseMapConsistencyTest) {
    int width = 10, height = 15;
    auto noiseMap1 =
        perlin.generateNoiseMap(width, height, 1.0, 4, 0.5, 2.0, 42);
    auto noiseMap2 =
        perlin.generateNoiseMap(width, height, 1.0, 4, 0.5, 2.0, 42);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            EXPECT_DOUBLE_EQ(noiseMap1[y][x], noiseMap2[y][x]);
        }
    }
}

TEST_F(PerlinNoiseTest, DifferentSeedsProduceDifferentMaps) {
    int width = 10, height = 15;
    auto noiseMap1 =
        perlin.generateNoiseMap(width, height, 1.0, 4, 0.5, 2.0, 42);
    auto noiseMap2 =
        perlin.generateNoiseMap(width, height, 1.0, 4, 0.5, 2.0, 43);
    bool different = false;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (noiseMap1[y][x] != noiseMap2[y][x]) {
                different = true;
                break;
            }
        }
        if (different)
            break;
    }
    EXPECT_TRUE(different);
}