#include "atom/algorithm/weight.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;
using namespace atom::utils;

// 为了测试方便，定义一个固定种子的Random类
template <typename T>
class FixedRandom
    : public Random<std::mt19937, std::uniform_real_distribution<T>> {
public:
    FixedRandom()
        : Random<std::mt19937, std::uniform_real_distribution<T>>(0.0, 1.0) {
        this->seed(12345);  // 使用固定的种子
    }
};

class WeightSelectorTest : public ::testing::Test {
protected:
    std::vector<double> weights{1.0, 2.0, 3.0, 4.0};
    WeightSelector<double> selector{weights};
};

TEST_F(WeightSelectorTest, Initialization) {
    EXPECT_EQ(selector.size(), 4);
    EXPECT_EQ(selector.getTotalWeight(), 10.0);
}

TEST_F(WeightSelectorTest, Selection) {
    std::map<size_t, int> selectionCounts;
    int numSelections = 10000;

    for (int i = 0; i < numSelections; ++i) {
        selectionCounts[selector.select()]++;
    }

    // 检查选择的分布是否大致符合权重
    EXPECT_NEAR(static_cast<double>(selectionCounts[0]) / numSelections, 0.1,
                0.02);
    EXPECT_NEAR(static_cast<double>(selectionCounts[1]) / numSelections, 0.2,
                0.02);
    EXPECT_NEAR(static_cast<double>(selectionCounts[2]) / numSelections, 0.3,
                0.02);
    EXPECT_NEAR(static_cast<double>(selectionCounts[3]) / numSelections, 0.4,
                0.02);
}

TEST_F(WeightSelectorTest, UpdateWeight) {
    selector.updateWeight(1, 5.0);
    EXPECT_EQ(selector.getWeight(1).value(), 5.0);
    EXPECT_EQ(selector.getTotalWeight(), 13.0);
}

TEST_F(WeightSelectorTest, AddWeight) {
    selector.addWeight(5.0);
    EXPECT_EQ(selector.size(), 5);
    EXPECT_EQ(selector.getTotalWeight(), 15.0);
}

TEST_F(WeightSelectorTest, RemoveWeight) {
    selector.removeWeight(1);
    EXPECT_EQ(selector.size(), 3);
    EXPECT_EQ(selector.getTotalWeight(), 8.0);
}

TEST_F(WeightSelectorTest, NormalizeWeights) {
    selector.normalizeWeights();
    EXPECT_NEAR(selector.getWeight(0).value(), 0.1, 1e-6);
    EXPECT_NEAR(selector.getWeight(1).value(), 0.2, 1e-6);
    EXPECT_NEAR(selector.getWeight(2).value(), 0.3, 1e-6);
    EXPECT_NEAR(selector.getWeight(3).value(), 0.4, 1e-6);
}

TEST_F(WeightSelectorTest, ApplyFunctionToWeights) {
    selector.applyFunctionToWeights([](double w) { return w * 2; });
    EXPECT_EQ(selector.getWeight(0).value(), 2.0);
    EXPECT_EQ(selector.getWeight(1).value(), 4.0);
    EXPECT_EQ(selector.getWeight(2).value(), 6.0);
    EXPECT_EQ(selector.getWeight(3).value(), 8.0);
}

TEST_F(WeightSelectorTest, BatchUpdateWeights) {
    std::vector<std::pair<size_t, double>> updates{{0, 10.0}, {2, 30.0}};
    selector.batchUpdateWeights(updates);
    EXPECT_EQ(selector.getWeight(0).value(), 10.0);
    EXPECT_EQ(selector.getWeight(2).value(), 30.0);
}

TEST_F(WeightSelectorTest, GetMaxWeightIndex) {
    EXPECT_EQ(selector.getMaxWeightIndex(), 3);
}

TEST_F(WeightSelectorTest, GetMinWeightIndex) {
    EXPECT_EQ(selector.getMinWeightIndex(), 0);
}

TEST_F(WeightSelectorTest, CustomSelectionStrategy) {
    auto customStrategy = std::make_unique<TopHeavySelectionStrategy<double>>();
    selector.setSelectionStrategy(std::move(customStrategy));

    std::map<size_t, int> selectionCounts;
    int numSelections = 10000;

    for (int i = 0; i < numSelections; ++i) {
        selectionCounts[selector.select()]++;
    }

    // 检查选择是否偏向较大的权重
    EXPECT_LT(static_cast<double>(selectionCounts[0]) / numSelections, 0.1);
    EXPECT_LT(static_cast<double>(selectionCounts[1]) / numSelections, 0.2);
    EXPECT_GT(static_cast<double>(selectionCounts[2]) / numSelections, 0.3);
    EXPECT_GT(static_cast<double>(selectionCounts[3]) / numSelections, 0.4);
}