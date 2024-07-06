#include "atom/search/search.hpp"
#include <gtest/gtest.h>

TEST(SearchEngineTest, AddDocumentTest) {
    atom::search::SearchEngine engine;
    atom::search::Document doc("1", "content", {"tag1", "tag2"});
    engine.addDocument(doc);

    // Test if the document is added correctly
    ASSERT_EQ(engine.searchByTag("tag1").size(), 1);
    ASSERT_EQ(engine.searchByTag("tag2").size(), 1);
    ASSERT_EQ(engine.searchByContent("content").size(), 1);
}

TEST(SearchEngineTest, SearchByTagTest) {
    atom::search::SearchEngine engine;
    engine.addDocument(
        atom::search::Document("1", "content1", {"tag1", "tag2"}));
    engine.addDocument(
        atom::search::Document("2", "content2", {"tag2", "tag3"}));
    engine.addDocument(
        atom::search::Document("3", "content3", {"tag3", "tag4"}));

    // Test exact search by tag
    ASSERT_EQ(engine.searchByTag("tag1").size(), 1);
    ASSERT_EQ(engine.searchByTag("tag2").size(), 2);
    ASSERT_EQ(engine.searchByTag("tag3").size(), 2);
    ASSERT_EQ(engine.searchByTag("tag4").size(), 1);

    // Test fuzzy search by tag
    ASSERT_EQ(engine.fuzzySearchByTag("tag1", 1).size(), 1);
    ASSERT_EQ(engine.fuzzySearchByTag("tag2", 1).size(), 2);
    ASSERT_EQ(engine.fuzzySearchByTag("tag3", 1).size(), 2);
    ASSERT_EQ(engine.fuzzySearchByTag("tag4", 1).size(), 1);
    ASSERT_EQ(engine.fuzzySearchByTag("tag5", 1).size(), 0);
}

TEST(SearchEngineTest, SearchByTagsTest) {
    atom::search::SearchEngine engine;
    engine.addDocument(
        atom::search::Document("1", "content1", {"tag1", "tag2"}));
    engine.addDocument(
        atom::search::Document("2", "content2", {"tag2", "tag3"}));
    engine.addDocument(
        atom::search::Document("3", "content3", {"tag3", "tag4"}));

    // Test search by multiple tags
    ASSERT_EQ(engine.searchByTags({"tag1", "tag2"}).size(), 1);
    ASSERT_EQ(engine.searchByTags({"tag2", "tag3"}).size(), 2);
    ASSERT_EQ(engine.searchByTags({"tag3", "tag4"}).size(), 1);
    ASSERT_EQ(engine.searchByTags({"tag1", "tag3"}).size(), 0);
}

TEST(SearchEngineTest, SearchByContentTest) {
    atom::search::SearchEngine engine;
    engine.addDocument(
        atom::search::Document("1", "content1", {"tag1", "tag2"}));
    engine.addDocument(
        atom::search::Document("2", "content2", {"tag2", "tag3"}));
    engine.addDocument(
        atom::search::Document("3", "content3", {"tag3", "tag4"}));

    // Test search by content
    ASSERT_EQ(engine.searchByContent("content1").size(), 1);
    ASSERT_EQ(engine.searchByContent("content2").size(), 1);
    ASSERT_EQ(engine.searchByContent("content3").size(), 1);
    ASSERT_EQ(engine.searchByContent("content4").size(), 0);
}

TEST(SearchEngineTest, BooleanSearchTest) {
    atom::search::SearchEngine engine;
    engine.addDocument(
        atom::search::Document("1", "content1 tag1 tag2", {"tag1", "tag2"}));
    engine.addDocument(
        atom::search::Document("2", "content2 tag2 tag3", {"tag2", "tag3"}));
    engine.addDocument(
        atom::search::Document("3", "content3 tag3 tag4", {"tag3", "tag4"}));

    // Test boolean search
    ASSERT_EQ(engine.booleanSearch("tag1 AND tag2").size(), 1);
    ASSERT_EQ(engine.booleanSearch("tag2 AND tag3").size(), 1);
    ASSERT_EQ(engine.booleanSearch("tag3 AND tag4").size(), 1);
    ASSERT_EQ(engine.booleanSearch("tag1 AND tag3").size(), 0);
    ASSERT_EQ(engine.booleanSearch("tag1 OR tag2").size(), 2);
    ASSERT_EQ(engine.booleanSearch("tag2 OR tag3").size(), 2);
    ASSERT_EQ(engine.booleanSearch("tag3 OR tag4").size(), 2);
    ASSERT_EQ(engine.booleanSearch("tag1 OR tag4").size(), 1);
    ASSERT_EQ(engine.booleanSearch("tag1 AND NOT tag2").size(), 0);
    ASSERT_EQ(engine.booleanSearch("tag2 AND NOT tag1").size(), 1);
    ASSERT_EQ(engine.booleanSearch("tag2 AND NOT tag3").size(), 0);
}

TEST(SearchEngineTest, AutoCompleteTest) {
    atom::search::SearchEngine engine;
    engine.addDocument(
        atom::search::Document("1", "content1", {"tag1", "tag2"}));
    engine.addDocument(
        atom::search::Document("2", "content2", {"tag2", "tag3"}));
    engine.addDocument(
        atom::search::Document("3", "content3", {"tag3", "tag4"}));

    // Test auto complete
    ASSERT_EQ(engine.autoComplete("con").size(), 3);
    ASSERT_EQ(engine.autoComplete("tag").size(), 4);
    ASSERT_EQ(engine.autoComplete("te").size(), 0);
}