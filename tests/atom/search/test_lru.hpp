#ifndef ATOM_SEARCH_TEST_LRU_HPP
#define ATOM_SEARCH_TEST_LRU_HPP

#include "atom/search/lru.hpp"

#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace atom::search;

class ThreadSafeLRUCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = std::make_unique<ThreadSafeLRUCache<std::string, int>>(3);
    }

    void TearDown() override { cache.reset(); }

    std::unique_ptr<ThreadSafeLRUCache<std::string, int>> cache;
};

TEST_F(ThreadSafeLRUCacheTest, PutAndGet) {
    cache->put("key1", 1);
    auto value = cache->get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);
}

TEST_F(ThreadSafeLRUCacheTest, GetNonExistentKey) {
    auto value = cache->get("key1");
    EXPECT_FALSE(value.has_value());
}

TEST_F(ThreadSafeLRUCacheTest, PutUpdatesValue) {
    cache->put("key1", 1);
    cache->put("key1", 2);
    auto value = cache->get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);
}

TEST_F(ThreadSafeLRUCacheTest, Erase) {
    cache->put("key1", 1);
    cache->erase("key1");
    auto value = cache->get("key1");
    EXPECT_FALSE(value.has_value());
}

TEST_F(ThreadSafeLRUCacheTest, Clear) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    cache->clear();
    EXPECT_EQ(cache->size(), 0);
}

TEST_F(ThreadSafeLRUCacheTest, Keys) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    auto keys = cache->keys();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_NE(std::find(keys.begin(), keys.end(), "key1"), keys.end());
    EXPECT_NE(std::find(keys.begin(), keys.end(), "key2"), keys.end());
}

TEST_F(ThreadSafeLRUCacheTest, PopLru) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    auto lru = cache->popLru();
    ASSERT_TRUE(lru.has_value());
    EXPECT_EQ(lru->first, "key1");
    EXPECT_EQ(lru->second, 1);
}

TEST_F(ThreadSafeLRUCacheTest, Resize) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    cache->put("key3", 3);
    cache->resize(2);
    EXPECT_EQ(cache->size(), 2);
    EXPECT_FALSE(cache->get("key1").has_value());
}

TEST_F(ThreadSafeLRUCacheTest, LoadFactor) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    EXPECT_FLOAT_EQ(cache->loadFactor(), 2.0 / 3.0);
}

TEST_F(ThreadSafeLRUCacheTest, HitRate) {
    cache->put("key1", 1);
    cache->get("key1");
    cache->get("key2");
    EXPECT_FLOAT_EQ(cache->hitRate(), 0.5);
}

TEST_F(ThreadSafeLRUCacheTest, SaveToFile) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    cache->saveToFile("test_cache.dat");

    auto newCache = std::make_unique<ThreadSafeLRUCache<std::string, int>>(3);
    newCache->loadFromFile("test_cache.dat");
    EXPECT_EQ(newCache->size(), 2);
    EXPECT_EQ(newCache->get("key1").value(), 1);
    EXPECT_EQ(newCache->get("key2").value(), 2);
}

TEST_F(ThreadSafeLRUCacheTest, LoadFromFile) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    cache->saveToFile("test_cache.dat");

    auto newCache = std::make_unique<ThreadSafeLRUCache<std::string, int>>(3);
    newCache->loadFromFile("test_cache.dat");
    EXPECT_EQ(newCache->size(), 2);
    EXPECT_EQ(newCache->get("key1").value(), 1);
    EXPECT_EQ(newCache->get("key2").value(), 2);
}

TEST_F(ThreadSafeLRUCacheTest, Expiry) {
    cache->put("key1", 1, std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_FALSE(cache->get("key1").has_value());
}

TEST_F(ThreadSafeLRUCacheTest, InsertCallback) {
    bool callbackCalled = false;
    cache->setInsertCallback([&callbackCalled](const std::string&, const int&) {
        callbackCalled = true;
    });
    cache->put("key1", 1);
    EXPECT_TRUE(callbackCalled);
}

TEST_F(ThreadSafeLRUCacheTest, EraseCallback) {
    bool callbackCalled = false;
    cache->setEraseCallback(
        [&callbackCalled](const std::string&) { callbackCalled = true; });
    cache->put("key1", 1);
    cache->erase("key1");
    EXPECT_TRUE(callbackCalled);
}

TEST_F(ThreadSafeLRUCacheTest, ClearCallback) {
    bool callbackCalled = false;
    cache->setClearCallback([&callbackCalled]() { callbackCalled = true; });
    cache->put("key1", 1);
    cache->clear();
    EXPECT_TRUE(callbackCalled);
}

#endif  // ATOM_SEARCH_TEST_LRU_HPP