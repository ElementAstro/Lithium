#ifndef ATOM_SEARCH_TEST_CACHE_HPP
#define ATOM_SEARCH_TEST_CACHE_HPP

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "cache.hpp"

using namespace atom::search;

class ResourceCacheTest : public ::testing::Test {
protected:
    void SetUp() override { cache = std::make_unique<ResourceCache<int>>(5); }

    void TearDown() override { cache.reset(); }

    std::unique_ptr<ResourceCache<int>> cache;
};

TEST_F(ResourceCacheTest, InsertAndGet) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    auto value = cache->get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);
}

TEST_F(ResourceCacheTest, Contains) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    EXPECT_TRUE(cache->contains("key1"));
    EXPECT_FALSE(cache->contains("key2"));
}

TEST_F(ResourceCacheTest, Remove) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->remove("key1");
    EXPECT_FALSE(cache->contains("key1"));
}

TEST_F(ResourceCacheTest, AsyncGet) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    auto future = cache->asyncGet("key1");
    auto value = future.get();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);
}

TEST_F(ResourceCacheTest, AsyncInsert) {
    auto future = cache->asyncInsert("key1", 1, std::chrono::seconds(10));
    future.get();
    EXPECT_TRUE(cache->contains("key1"));
}

TEST_F(ResourceCacheTest, Clear) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->clear();
    EXPECT_FALSE(cache->contains("key1"));
}

TEST_F(ResourceCacheTest, Size) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->insert("key2", 2, std::chrono::seconds(10));
    EXPECT_EQ(cache->size(), 2);
}

TEST_F(ResourceCacheTest, Empty) {
    EXPECT_TRUE(cache->empty());
    cache->insert("key1", 1, std::chrono::seconds(10));
    EXPECT_FALSE(cache->empty());
}

TEST_F(ResourceCacheTest, EvictOldest) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->insert("key2", 2, std::chrono::seconds(10));
    cache->insert("key3", 3, std::chrono::seconds(10));
    cache->insert("key4", 4, std::chrono::seconds(10));
    cache->insert("key5", 5, std::chrono::seconds(10));
    cache->insert("key6", 6, std::chrono::seconds(10));
    EXPECT_FALSE(cache->contains("key1"));
    EXPECT_TRUE(cache->contains("key6"));
}

TEST_F(ResourceCacheTest, IsExpired) {
    cache->insert("key1", 1, std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(cache->isExpired("key1"));
}

TEST_F(ResourceCacheTest, AsyncLoad) {
    auto future = cache->asyncLoad("key1", []() { return 1; });
    future.get();
    EXPECT_TRUE(cache->contains("key1"));
}

TEST_F(ResourceCacheTest, SetMaxSize) {
    cache->setMaxSize(2);
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->insert("key2", 2, std::chrono::seconds(10));
    cache->insert("key3", 3, std::chrono::seconds(10));
    EXPECT_FALSE(cache->contains("key1"));
    EXPECT_TRUE(cache->contains("key3"));
}

TEST_F(ResourceCacheTest, SetExpirationTime) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->setExpirationTime("key1", std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(cache->isExpired("key1"));
}

TEST_F(ResourceCacheTest, InsertBatch) {
    std::vector<std::pair<std::string, int>> items = {{"key1", 1}, {"key2", 2}};
    cache->insertBatch(items, std::chrono::seconds(10));
    EXPECT_TRUE(cache->contains("key1"));
    EXPECT_TRUE(cache->contains("key2"));
}

TEST_F(ResourceCacheTest, RemoveBatch) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->insert("key2", 2, std::chrono::seconds(10));
    std::vector<std::string> keys = {"key1", "key2"};
    cache->removeBatch(keys);
    EXPECT_FALSE(cache->contains("key1"));
    EXPECT_FALSE(cache->contains("key2"));
}

TEST_F(ResourceCacheTest, GetStatistics) {
    cache->insert("key1", 1, std::chrono::seconds(10));
    cache->get("key1");
    cache->get("key2");
    auto [hits, misses] = cache->getStatistics();
    EXPECT_EQ(hits, 1);
    EXPECT_EQ(misses, 1);
}

#endif  // ATOM_SEARCH_TEST_CACHE_HPP