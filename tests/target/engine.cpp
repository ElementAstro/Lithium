#include <gtest/gtest.h>

#include "target/engine.hpp"

using namespace lithium::target;

class StarObjectSearchTest : public ::testing::Test {
protected:
    void SetUp() override {
        searchEngine.addStarObject(
            {"Sirius", {"Dog Star", "Alpha Canis Majoris"}});
        searchEngine.addStarObject({"Betelgeuse", {"Alpha Orionis"}});
        searchEngine.addStarObject({"Vega", {"Alpha Lyrae"}});
    }

    SearchEngine searchEngine;
};

// 测试按名称或别名搜索
TEST_F(StarObjectSearchTest, SearchByNameOrAlias) {
    auto results = searchEngine.searchStarObject("Sirius");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "Sirius");

    results = searchEngine.searchStarObject("Dog Star");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "Sirius");

    results = searchEngine.searchStarObject("Alpha Orionis");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "Betelgeuse");
}

// 测试模糊搜索
TEST_F(StarObjectSearchTest, FuzzySearchStarObject) {
    auto results =
        searchEngine.fuzzySearchStarObject("Sirious", 2);  // Tolerance of 2
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "Sirius");

    results = searchEngine.fuzzySearchStarObject("Alpha Orionis",
                                                 1);  // Tolerance of 1
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "Betelgeuse");

    results =
        searchEngine.fuzzySearchStarObject("Apha Lyrae", 1);  // Tolerance of 1
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "Vega");
}

// 测试自动补全
TEST_F(StarObjectSearchTest, AutoCompleteStarObject) {
    auto suggestions = searchEngine.autoCompleteStarObject("Alp");
    ASSERT_EQ(suggestions.size(), 3);
    EXPECT_TRUE(std::ranges::any_of(
        suggestions, [](const auto& s) { return s == "Alpha Canis Majoris"; }));
    EXPECT_TRUE(std::ranges::any_of(
        suggestions, [](const auto& s) { return s == "Alpha Orionis"; }));
    EXPECT_TRUE(std::ranges::any_of(
        suggestions, [](const auto& s) { return s == "Alpha Lyrae"; }));

    suggestions = searchEngine.autoCompleteStarObject("Bet");
    ASSERT_EQ(suggestions.size(), 1);
    EXPECT_EQ(suggestions[0], "Betelgeuse");

    suggestions = searchEngine.autoCompleteStarObject("V");
    ASSERT_EQ(suggestions.size(), 1);
    EXPECT_EQ(suggestions[0], "Vega");
}

// 测试按点击量排序
TEST_F(StarObjectSearchTest, RankedResults) {
    auto results = searchEngine.searchStarObject("Sirius");
    results[0].clickCount = 10;
    results = searchEngine.searchStarObject("Betelgeuse");
    results[0].clickCount = 5;
    results = searchEngine.searchStarObject("Vega");
    results[0].clickCount = 15;

    std::vector<StarObject> allResults = {StarObject{"Sirius", {}, 10},
                                          StarObject{"Betelgeuse", {}, 5},
                                          StarObject{"Vega", {}, 15}};
    auto rankedResults = searchEngine.getRankedResults(allResults);

    ASSERT_EQ(rankedResults.size(), 3);
    EXPECT_EQ(rankedResults[0].name, "Vega");
    EXPECT_EQ(rankedResults[1].name, "Sirius");
    EXPECT_EQ(rankedResults[2].name, "Betelgeuse");
}
