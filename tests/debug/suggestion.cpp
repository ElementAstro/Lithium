#include "debug/suggestion.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace lithium::debug;
using ::testing::Return;

// Mock class for SuggestionEngine
class MockSuggestionEngine : public SuggestionEngine {
public:
    MockSuggestionEngine(const std::vector<std::string>& dataset,
                         int maxSuggestions = 5)
        : SuggestionEngine(dataset, maxSuggestions) {}

    MOCK_METHOD(void, buildIndex, ());
    MOCK_METHOD(bool, matches,
                (const std::string&, const std::string&, MatchType));
    MOCK_METHOD(int, calculateScore, (const std::string&, const std::string&));
};

class SuggestionEngineTest : public ::testing::Test {
protected:
    std::vector<std::string> dataset{"apple", "banana", "grape", "orange",
                                     "watermelon"};
    SuggestionEngine engine{dataset, 3};

    void SetUp() override { engine = SuggestionEngine(dataset, 3); }
};

TEST_F(SuggestionEngineTest, SuggestPrefixMatch) {
    auto suggestions =
        engine.suggest("ap", SuggestionEngine::MatchType::Prefix);
    std::vector<std::string> expected{"apple"};
    EXPECT_EQ(suggestions, expected);
}

TEST_F(SuggestionEngineTest, SuggestSubstringMatch) {
    auto suggestions =
        engine.suggest("an", SuggestionEngine::MatchType::Substring);
    std::vector<std::string> expected{"banana", "orange"};
    EXPECT_EQ(suggestions, expected);
}

TEST_F(SuggestionEngineTest, SuggestLimitedResults) {
    auto suggestions =
        engine.suggest("a", SuggestionEngine::MatchType::Substring);
    std::vector<std::string> expected{"banana", "grape", "orange"};
    EXPECT_EQ(suggestions, expected);
}

TEST_F(SuggestionEngineTest, SuggestCaseInsensitive) {
    auto suggestions =
        engine.suggest("Ap", SuggestionEngine::MatchType::Prefix);
    std::vector<std::string> expected{"apple"};
    EXPECT_EQ(suggestions, expected);
}

TEST(SuggestionEngineMockTest, MockMatches) {
    std::vector<std::string> dataset{"apple", "banana", "grape"};
    MockSuggestionEngine mockEngine{dataset, 3};

    EXPECT_CALL(mockEngine,
                matches("ap", "apple", SuggestionEngine::MatchType::Prefix))
        .WillOnce(Return(true));
    EXPECT_CALL(mockEngine,
                matches("ap", "banana", SuggestionEngine::MatchType::Prefix))
        .WillOnce(Return(false));
    EXPECT_CALL(mockEngine,
                matches("ap", "grape", SuggestionEngine::MatchType::Prefix))
        .WillOnce(Return(false));

    auto suggestions =
        mockEngine.suggest("ap", SuggestionEngine::MatchType::Prefix);
    std::vector<std::string> expected{"apple"};
    EXPECT_EQ(suggestions, expected);
}

TEST(SuggestionEngineMockTest, MockCalculateScore) {
    std::vector<std::string> dataset{"apple", "banana", "grape"};
    MockSuggestionEngine mockEngine{dataset, 3};

    EXPECT_CALL(mockEngine, matches(testing::_, testing::_,
                                    SuggestionEngine::MatchType::Prefix))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(mockEngine, calculateScore("ap", "apple")).WillOnce(Return(5));
    EXPECT_CALL(mockEngine, calculateScore("ap", "banana")).WillOnce(Return(2));
    EXPECT_CALL(mockEngine, calculateScore("ap", "grape")).WillOnce(Return(3));

    auto suggestions =
        mockEngine.suggest("ap", SuggestionEngine::MatchType::Prefix);
    std::vector<std::string> expected{"apple", "grape", "banana"};
    EXPECT_EQ(suggestions, expected);
}
