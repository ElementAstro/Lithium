#ifndef ATOM_SEARCH_TEST_TTL_HPP
#define ATOM_SEARCH_TEST_TTL_HPP

#include "atom/search/ttl.hpp"

#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace atom::search;

class TTLCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = std::make_unique<TTLCache<std::string, int>>(
            std::chrono::milliseconds(100), 3);
    }

    void TearDown() override { cache.reset(); }

    std::unique_ptr<TTLCache<std::string, int>> cache;
};

TEST_F(TTLCacheTest, PutAndGet) {
    cache->put("key1", 1);
    auto value = cache->get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);
}

TEST_F(TTLCacheTest, GetNonExistentKey) {
    auto value = cache->get("key1");
    EXPECT_FALSE(value.has_value());
}

TEST_F(TTLCacheTest, PutUpdatesValue) {
    cache->put("key1", 1);
    cache->put("key1", 2);
    auto value = cache->get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);
}

TEST_F(TTLCacheTest, Expiry) {
    cache->put("key1", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto value = cache->get("key1");
    EXPECT_FALSE(value.has_value());
}

TEST_F(TTLCacheTest, Cleanup) {
    cache->put("key1", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cache->cleanup();
    EXPECT_EQ(cache->size(), 0);
}

TEST_F(TTLCacheTest, HitRate) {
    cache->put("key1", 1);
    cache->get("key1");
    cache->get("key2");
    EXPECT_DOUBLE_EQ(cache->hitRate(), 0.5);
}

TEST_F(TTLCacheTest, Size) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    EXPECT_EQ(cache->size(), 2);
}

TEST_F(TTLCacheTest, Clear) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    cache->clear();
    EXPECT_EQ(cache->size(), 0);
}

TEST_F(TTLCacheTest, LRU_Eviction) {
    cache->put("key1", 1);
    cache->put("key2", 2);
    cache->put("key3", 3);
    cache->put("key4", 4);  // This should evict "key1"
    EXPECT_FALSE(cache->get("key1").has_value());
    EXPECT_TRUE(cache->get("key4").has_value());
}

#endif  // ATOM_SEARCH_TEST_TTL_HPP
