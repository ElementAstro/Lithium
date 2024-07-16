#include <gtest/gtest.h>

#include <fstream>

#include "atom/system/crash_quotes.hpp"
#include "atom/type/json.hpp"

using namespace atom::system;
using json = nlohmann::json;
namespace fs = std::filesystem;

// Helper function to create a sample quote
auto createSampleQuote() -> Quote {
    return Quote("To be or not to be", "William Shakespeare");
}

// Helper function to create a JSON file with quotes
void createSampleJsonFile(const std::string& filename) {
    std::ofstream file(filename);
    json data = {
        {{"text", "To be or not to be"}, {"author", "William Shakespeare"}},
        {{"text", "I think, therefore I am"}, {"author", "René Descartes"}},
        {{"text", "The unexamined life is not worth living"},
         {"author", "Socrates"}}};
    file << data.dump(4);
    file.close();
}

// Test fixture for setting up common test environment
class QuoteManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
        fs::remove("sample_quotes.json");
    }

    QuoteManager manager;
};

// Test addQuote function
TEST_F(QuoteManagerTest, AddQuote) {
    Quote quote = createSampleQuote();
    manager.addQuote(quote);
    auto quotes = manager.searchQuotes("To be or not to be");
    ASSERT_EQ(quotes.size(), 1);
    EXPECT_EQ(quotes[0].getText(), "To be or not to be");
    EXPECT_EQ(quotes[0].getAuthor(), "William Shakespeare");
}

// Test removeQuote function
TEST_F(QuoteManagerTest, RemoveQuote) {
    Quote quote = createSampleQuote();
    manager.addQuote(quote);
    manager.removeQuote(quote);
    auto quotes = manager.searchQuotes("To be or not to be");
    EXPECT_TRUE(quotes.empty());
}

// Test shuffleQuotes function
TEST_F(QuoteManagerTest, ShuffleQuotes) {
    manager.addQuote(Quote("Quote 1", "Author 1"));
    manager.addQuote(Quote("Quote 2", "Author 2"));
    manager.addQuote(Quote("Quote 3", "Author 3"));
    manager.shuffleQuotes();
    // Since shuffling is random, we just check that all quotes are still
    // present
    EXPECT_EQ(manager.searchQuotes("Quote").size(), 3);
}

// Test clearQuotes function
TEST_F(QuoteManagerTest, ClearQuotes) {
    manager.addQuote(createSampleQuote());
    manager.clearQuotes();
    auto quotes = manager.searchQuotes("To be or not to be");
    EXPECT_TRUE(quotes.empty());
}

// Test loadQuotesFromJson function
TEST_F(QuoteManagerTest, LoadQuotesFromJson) {
    createSampleJsonFile("sample_quotes.json");
    manager.loadQuotesFromJson("sample_quotes.json");
    EXPECT_EQ(manager.searchQuotes("To be or not to be").size(), 1);
    EXPECT_EQ(manager.searchQuotes("I think, therefore I am").size(), 1);
    EXPECT_EQ(
        manager.searchQuotes("The unexamined life is not worth living").size(),
        1);
}

// Test saveQuotesToJson function
TEST_F(QuoteManagerTest, SaveQuotesToJson) {
    manager.addQuote(createSampleQuote());
    manager.saveQuotesToJson("sample_quotes.json");
    std::ifstream file("sample_quotes.json");
    json data;
    file >> data;
    file.close();
    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data[0]["text"], "To be or not to be");
    EXPECT_EQ(data[0]["author"], "William Shakespeare");
}

// Test searchQuotes function
TEST_F(QuoteManagerTest, SearchQuotes) {
    manager.addQuote(createSampleQuote());
    auto quotes = manager.searchQuotes("To be");
    ASSERT_EQ(quotes.size(), 1);
    EXPECT_EQ(quotes[0].getText(), "To be or not to be");
    EXPECT_EQ(quotes[0].getAuthor(), "William Shakespeare");
}

// Test filterQuotesByAuthor function
TEST_F(QuoteManagerTest, FilterQuotesByAuthor) {
    manager.addQuote(createSampleQuote());
    auto quotes = manager.filterQuotesByAuthor("William Shakespeare");
    ASSERT_EQ(quotes.size(), 1);
    EXPECT_EQ(quotes[0].getText(), "To be or not to be");
    EXPECT_EQ(quotes[0].getAuthor(), "William Shakespeare");
}

// Test getRandomQuote function
TEST_F(QuoteManagerTest, GetRandomQuote) {
    manager.addQuote(createSampleQuote());
    auto quote = manager.getRandomQuote();
    EXPECT_EQ(quote, "To be or not to be - William Shakespeare");
}