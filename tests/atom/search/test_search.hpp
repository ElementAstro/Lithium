#ifndef ATOM_SEARCH_TEST_SEARCH_HPP
#define ATOM_SEARCH_TEST_SEARCH_HPP

#include <gtest/gtest.h>

#include "atom/search/search.hpp"

using namespace atom::search;

// Test fixture for SearchEngine
class SearchEngineTest : public ::testing::Test {
protected:
    SearchEngine engine;

    void SetUp() override {
        // Add some initial documents to the search engine
        engine.addDocument(Document("1", "Hello world", {"greeting", "world"}));
        engine.addDocument(
            Document("2", "Goodbye world", {"farewell", "world"}));
    }
};

TEST_F(SearchEngineTest, AddDocument) {
    Document doc("3", "New document", {"new", "document"});
    engine.addDocument(doc);
    auto result = engine.searchByTag("new");
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].id, "3");
}

TEST_F(SearchEngineTest, RemoveDocument) {
    engine.removeDocument("1");
    ASSERT_THROW(engine.removeDocument("1"), DocumentNotFoundException);
}

TEST_F(SearchEngineTest, UpdateDocument) {
    Document updatedDoc("1", "Updated content", {"updated", "content"});
    engine.updateDocument(updatedDoc);
    auto result = engine.searchByTag("updated");
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].content, "Updated content");
}

TEST_F(SearchEngineTest, SearchByTag) {
    auto result = engine.searchByTag("world");
    ASSERT_EQ(result.size(), 2);
}

TEST_F(SearchEngineTest, FuzzySearchByTag) {
    auto result = engine.fuzzySearchByTag("wrold", 1);
    ASSERT_EQ(result.size(), 2);
}

TEST_F(SearchEngineTest, SearchByTags) {
    auto result = engine.searchByTags({"greeting", "world"});
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].id, "1");
}

TEST_F(SearchEngineTest, SearchByContent) {
    auto result = engine.searchByContent("Goodbye");
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].id, "2");
}

TEST_F(SearchEngineTest, BooleanSearch) {
    auto result = engine.booleanSearch("Hello AND world");
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].id, "1");
}

TEST_F(SearchEngineTest, AutoComplete) {
    auto suggestions = engine.autoComplete("wo");
    ASSERT_EQ(suggestions.size(), 1);
    ASSERT_EQ(suggestions[0], "world");
}

TEST_F(SearchEngineTest, SaveAndLoadIndex) {
    engine.saveIndex("test_index.json");
    SearchEngine newEngine;
    newEngine.loadIndex("test_index.json");
    auto result = newEngine.searchByTag("world");
    ASSERT_EQ(result.size(), 2);
}

#endif
