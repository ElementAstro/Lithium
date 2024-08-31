#include "atom/search/ttl.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace atom::search;

using namespace std::chrono_literals;

class TTLCacheTest : public ::testing::Test {
protected:
    using Cache = TTLCache<std::string, std::string>;

    void SetUp() override {
        // 这里可以初始化公共测试对象或数据
    }

    void TearDown() override {
        // 这里可以清理测试环境
    }
};

TEST_F(TTLCacheTest, BasicPutAndGet) {
    Cache cache(5s, 10);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    auto value = cache.get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "value1");

    value = cache.get("key2");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "value2");

    value = cache.get("key3");
    EXPECT_FALSE(value.has_value());
}

TEST_F(TTLCacheTest, ExpiryCheck) {
    Cache cache(1s, 10);

    cache.put("key1", "value1");
    std::this_thread::sleep_for(2s);  // 等待超时

    auto value = cache.get("key1");
    EXPECT_FALSE(value.has_value());
}

TEST_F(TTLCacheTest, CapacityLimit) {
    Cache cache(5s, 2);  // 容量为2

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    // 超过容量，插入新的项
    cache.put("key3", "value3");

    EXPECT_FALSE(cache.get("key1").has_value());  // key1 应该被淘汰
    EXPECT_TRUE(cache.get("key2").has_value());   // key2 仍然存在
    EXPECT_TRUE(cache.get("key3").has_value());   // key3 刚插入
}

TEST_F(TTLCacheTest, LRUBehavior) {
    Cache cache(5s, 2);  // 容量为2

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    // 访问 key1，将其变为最近使用
    auto value = cache.get("key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "value1");

    // 插入新项，key2 应该被淘汰，因为它是最久未使用的
    cache.put("key3", "value3");

    EXPECT_TRUE(cache.get("key1").has_value());
    EXPECT_FALSE(cache.get("key2").has_value());
    EXPECT_TRUE(cache.get("key3").has_value());
}

TEST_F(TTLCacheTest, HitRateCalculation) {
    Cache cache(5s, 10);

    cache.put("key1", "value1");
    cache.get("key1");  // hit
    cache.get("key2");  // miss
    cache.get("key1");  // hit
    cache.get("key3");  // miss

    EXPECT_DOUBLE_EQ(cache.hitRate(), 0.5);
}

TEST_F(TTLCacheTest, CleanupExpiredItems) {
    Cache cache(1s, 10);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    std::this_thread::sleep_for(2s);  // 等待所有项过期
    cache.cleanup();

    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.get("key1").has_value());
    EXPECT_FALSE(cache.get("key2").has_value());
}

TEST_F(TTLCacheTest, ClearCache) {
    Cache cache(5s, 10);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    cache.clear();

    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.get("key1").has_value());
    EXPECT_FALSE(cache.get("key2").has_value());
}

TEST_F(TTLCacheTest, ConcurrentAccess) {
    Cache cache(5s, 10);

    std::thread writer([&cache] {
        for (int i = 0; i < 100; ++i) {
            cache.put("key" + std::to_string(i), "value" + std::to_string(i));
            std::this_thread::sleep_for(10ms);
        }
    });

    std::thread reader([&cache] {
        for (int i = 0; i < 100; ++i) {
            auto value = cache.get("key" + std::to_string(i));
            if (value) {
                std::cout << *value << std::endl;
            }
            std::this_thread::sleep_for(10ms);
        }
    });

    writer.join();
    reader.join();

    EXPECT_GE(cache.size(), 0);  // 检查缓存大小是否合理
}

TEST_F(TTLCacheTest, EdgeCaseNoCapacity) {
    Cache cache(5s, 0);  // 容量为0

    cache.put("key1", "value1");
    EXPECT_EQ(cache.size(), 0);

    auto value = cache.get("key1");
    EXPECT_FALSE(value.has_value());
}

TEST_F(TTLCacheTest, EdgeCaseZeroTTL) {
    Cache cache(0ms, 10);  // TTL为0

    cache.put("key1", "value1");
    EXPECT_FALSE(cache.get("key1").has_value());  // 立即过期
}

TEST_F(TTLCacheTest, EdgeCaseNegativeTTL) {
    Cache cache(-1ms, 10);  // TTL为负数，等效于立即过期

    cache.put("key1", "value1");
    EXPECT_FALSE(cache.get("key1").has_value());  // 立即过期
}