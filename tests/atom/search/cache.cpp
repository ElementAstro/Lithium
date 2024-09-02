#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "atom/search/cache.hpp"

using namespace atom::search;

class ResourceCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache = new ResourceCache<int>(3);  // 最大缓存大小为 3
    }

    void TearDown() override { delete cache; }

    ResourceCache<int> *cache;
};

TEST_F(ResourceCacheTest, InsertAndGet) {
    cache->insert("a", 1, std::chrono::seconds(10));
    auto value = cache->get("a");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);
}

TEST_F(ResourceCacheTest, Expiration) {
    cache->insert("b", 2, std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto value = cache->get("b");
    EXPECT_FALSE(value.has_value());  // 元素应已过期
}

TEST_F(ResourceCacheTest, Eviction) {
    cache->insert("a", 1, std::chrono::seconds(10));
    cache->insert("b", 2, std::chrono::seconds(10));
    cache->insert("c", 3, std::chrono::seconds(10));
    cache->insert("d", 4, std::chrono::seconds(10));  // 插入新元素，应触发驱逐

    EXPECT_FALSE(cache->contains("a"));  // "a" 是最早插入的，应被驱逐
    EXPECT_TRUE(cache->contains("b"));
    EXPECT_TRUE(cache->contains("c"));
    EXPECT_TRUE(cache->contains("d"));
}

TEST_F(ResourceCacheTest, ClearCache) {
    cache->insert("a", 1, std::chrono::seconds(10));
    cache->insert("b", 2, std::chrono::seconds(10));
    cache->clear();

    EXPECT_EQ(cache->size(), 0);
    EXPECT_FALSE(cache->contains("a"));
    EXPECT_FALSE(cache->contains("b"));
}

TEST_F(ResourceCacheTest, AsyncInsertAndGet) {
    auto futureInsert = cache->asyncInsert("e", 5, std::chrono::seconds(10));
    futureInsert.wait();  // 等待异步插入完成

    auto futureGet = cache->asyncGet("e");
    auto value = futureGet.get();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 5);
}

TEST_F(ResourceCacheTest, BatchInsertAndRemove) {
    std::vector<std::pair<std::string, int>> items = {
        {"a", 1}, {"b", 2}, {"c", 3}};
    cache->insertBatch(items, std::chrono::seconds(10));

    EXPECT_TRUE(cache->contains("a"));
    EXPECT_TRUE(cache->contains("b"));
    EXPECT_TRUE(cache->contains("c"));

    cache->removeBatch({"a", "b"});
    EXPECT_FALSE(cache->contains("a"));
    EXPECT_FALSE(cache->contains("b"));
    EXPECT_TRUE(cache->contains("c"));
}

TEST_F(ResourceCacheTest, HandleDuplicateInserts) {
    cache->insert("a", 1, std::chrono::seconds(10));
    cache->insert("a", 2, std::chrono::seconds(10));  // 重复插入

    auto value = cache->get("a");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 2);  // 值应被更新为 2
}

TEST_F(ResourceCacheTest, ZeroCapacityCache) {
    ResourceCache<int> zeroCapacityCache(0);  // 测试容量为 0 的缓存
    zeroCapacityCache.insert("a", 1, std::chrono::seconds(10));
    EXPECT_EQ(zeroCapacityCache.size(), 0);  // 无法保存任何元素
    EXPECT_FALSE(zeroCapacityCache.contains("a"));
}

TEST_F(ResourceCacheTest, ConcurrentAccess) {
    std::vector<std::thread> threads;

    // 并发插入
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([this, i]() {
            cache->insert("key" + std::to_string(i), i,
                          std::chrono::seconds(5));
        });
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 并发获取
    threads.clear();
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([this, i]() {
            auto value = cache->get("key" + std::to_string(i));
            if (value.has_value()) {
                EXPECT_EQ(value.value(), i);
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }
}

TEST_F(ResourceCacheTest, LoadFromFile) {
    cache->insert("a", 1, std::chrono::seconds(10));
    cache->insert("b", 2, std::chrono::seconds(10));

    // 写入文件
    cache->writeToFile("cache_data.txt",
                       [](const int &value) { return std::to_string(value); });

    // 新建缓存并从文件加载
    ResourceCache<int> newCache(3);
    newCache.readFromFile("cache_data.txt", [](const std::string &str) {
        return std::stoi(str);
    });

    EXPECT_TRUE(newCache.contains("a"));
    EXPECT_TRUE(newCache.contains("b"));
    auto value = newCache.get("a");
    EXPECT_EQ(value.value(), 1);
}

TEST_F(ResourceCacheTest, LoadFromJsonFile) {
    cache->insert("a", 1, std::chrono::seconds(10));
    cache->insert("b", 2, std::chrono::seconds(10));

    // 写入 JSON 文件
    cache->writeToJsonFile("cache_data.json",
                           [](const int &value) { return json(value); });

    // 新建缓存并从 JSON 文件加载
    ResourceCache<int> newCache(3);
    newCache.readFromJsonFile("cache_data.json",
                              [](const json &j) { return j.get<int>(); });

    EXPECT_TRUE(newCache.contains("a"));
    EXPECT_TRUE(newCache.contains("b"));
    auto value = newCache.get("b");
    EXPECT_EQ(value.value(), 2);
}
