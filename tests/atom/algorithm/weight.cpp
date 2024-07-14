#include "atom/algorithm/weight.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

class WeightSelectorTest : public ::testing::Test {
protected:
    WeightSelector<double> selector;

    WeightSelectorTest() : selector(std::vector<double>{1.0, 2.0, 3.0, 4.0}) {}

    void SetUp() override {
        // Custom setup code can go here
    }
};

TEST_F(WeightSelectorTest, Initialization) {
    EXPECT_EQ(selector.size(), 4);
    EXPECT_DOUBLE_EQ(selector.getTotalWeight(), 10.0);
}

TEST_F(WeightSelectorTest, Selection) {
    size_t index = selector.select();
    EXPECT_LT(index, selector.size());
}

TEST_F(WeightSelectorTest, UpdateWeight) {
    selector.updateWeight(2, 5.0);
    EXPECT_DOUBLE_EQ(selector.getWeight(2).value(), 5.0);
    EXPECT_DOUBLE_EQ(selector.getTotalWeight(), 12.0);
}

TEST_F(WeightSelectorTest, AddWeight) {
    selector.addWeight(6.0);
    EXPECT_EQ(selector.size(), 5);
    EXPECT_DOUBLE_EQ(selector.getTotalWeight(), 16.0);
}

TEST_F(WeightSelectorTest, RemoveWeight) {
    selector.removeWeight(1);
    EXPECT_EQ(selector.size(), 3);
    EXPECT_DOUBLE_EQ(selector.getTotalWeight(), 8.0);
}

TEST_F(WeightSelectorTest, NormalizeWeights) {
    selector.normalizeWeights();
    double totalWeight = selector.getTotalWeight();
    EXPECT_DOUBLE_EQ(totalWeight, 1.0);
}

TEST_F(WeightSelectorTest, ApplyFunctionToWeights) {
    selector.applyFunctionToWeights([](double w) { return w * 2; });
    EXPECT_DOUBLE_EQ(selector.getTotalWeight(), 20.0);
}

TEST_F(WeightSelectorTest, BatchUpdateWeights) {
    selector.batchUpdateWeights({{0, 4.0}, {2, 5.0}});
    EXPECT_DOUBLE_EQ(selector.getWeight(0).value(), 4.0);
    EXPECT_DOUBLE_EQ(selector.getWeight(2).value(), 5.0);
    EXPECT_DOUBLE_EQ(selector.getTotalWeight(), 15.0);
}

TEST_F(WeightSelectorTest, GetWeight) {
    EXPECT_DOUBLE_EQ(selector.getWeight(1).value(), 2.0);
    EXPECT_EQ(selector.getWeight(5), std::nullopt);
}

TEST_F(WeightSelectorTest, GetMaxWeightIndex) {
    EXPECT_EQ(selector.getMaxWeightIndex(), 3);
}

TEST_F(WeightSelectorTest, GetMinWeightIndex) {
    EXPECT_EQ(selector.getMinWeightIndex(), 0);
}

TEST_F(WeightSelectorTest, PrintWeights) {
    std::ostringstream oss;
    selector.printWeights(oss);
    EXPECT_EQ(oss.str(), "[1.00, 2.00, 3.00, 4.00]\n");
}

class TopHeavySelectionStrategyTest : public ::testing::Test {
protected:
    WeightSelector<double> selector;

    TopHeavySelectionStrategyTest()
        : selector(std::vector<double>{1.0, 2.0, 3.0, 4.0},
                   std::make_unique<TopHeavySelectionStrategy<double>>()) {}

    void SetUp() override {
        // Custom setup code can go here
    }
};

TEST_F(TopHeavySelectionStrategyTest, TopHeavySelection) {
    size_t index = selector.select();
    EXPECT_LT(index, selector.size());
}