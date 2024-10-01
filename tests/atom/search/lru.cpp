#include <gtest/gtest.h>
#include <thread>

#include "atom/search/lru.hpp"

using namespace atom::search;

TEST(LRUCacheTest, BasicPutAndGet) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    EXPECT_EQ(cache.get(1).value_or("not found"), "one");
    EXPECT_EQ(cache.get(2).value_or("not found"), "two");
    EXPECT_EQ(cache.get(3).value_or("not found"), "three");
}

// 测试缓存的LRU行为
TEST(LRUCacheTest, LRUBehavior) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");
    cache.put(4, "four");  // 这将导致移除最早的键 1

    EXPECT_EQ(cache.get(1).value_or("not found"), "not found");  // 1 应该被移除
    EXPECT_EQ(cache.get(2).value_or("not found"), "two");  // 2 应该仍然存在
}

// 测试删除功能
TEST(LRUCacheTest, Erase) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    cache.erase(2);                                              // 删除键 2
    EXPECT_EQ(cache.get(2).value_or("not found"), "not found");  // 2 应该被移除
    EXPECT_EQ(cache.get(1).value_or("not found"), "one");
    EXPECT_EQ(cache.get(3).value_or("not found"), "three");
}

// 测试清空缓存
TEST(LRUCacheTest, ClearCache) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    cache.clear();  // 清空缓存
    EXPECT_EQ(cache.size(), 0);
    EXPECT_EQ(cache.get(1).value_or("not found"), "not found");
    EXPECT_EQ(cache.get(2).value_or("not found"), "not found");
}

// 测试缓存命中率
TEST(LRUCacheTest, HitRate) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");

    cache.get(1);  // 命中
    cache.get(3);  // 未命中

    EXPECT_FLOAT_EQ(cache.hitRate(), 0.5);  // 命中率应该是 50%
}

// 测试回调函数
TEST(LRUCacheTest, Callbacks) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    bool insertCalled = false;
    bool eraseCalled = false;
    bool clearCalled = false;

    cache.setInsertCallback([&insertCalled](int key, const std::string& value) {
        (void)key;    // 避免未使用参数警告
        (void)value;  // 避免未使用参数警告
        insertCalled = true;
    });

    cache.setEraseCallback([&eraseCalled](int key) {
        (void)key;  // 避免未使用参数警告
        eraseCalled = true;
    });

    cache.setClearCallback([&clearCalled]() { clearCalled = true; });

    cache.put(1, "one");
    EXPECT_TRUE(insertCalled);

    cache.erase(1);
    EXPECT_TRUE(eraseCalled);

    cache.clear();
    EXPECT_TRUE(clearCalled);
}

// 测试过期功能
TEST(LRUCacheTest, Expiry) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one", std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 等待缓存项过期

    EXPECT_EQ(cache.get(1).value_or("not found"),
              "not found");  // 1 应该已过期并被移除
}

// 测试持久化功能
TEST(LRUCacheTest, Persistence) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");

    std::string filename = "cache_data.dat";
    cache.saveToFile(filename);  // 保存到文件

    // 加载到新的缓存实例中
    ThreadSafeLRUCache<int, std::string> cache2(3);
    cache2.loadFromFile(filename);

    EXPECT_EQ(cache2.get(1).value_or("not found"), "one");
    EXPECT_EQ(cache2.get(2).value_or("not found"), "two");
}

// 测试边缘情况: 缓存为空时调用 pop_lru
TEST(LRUCacheTest, PopLRUOnEmptyCache) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    auto result = cache.popLru();
    EXPECT_FALSE(result.has_value());  // 应该没有返回值
}

// 测试边缘情况: 在缓存已满时进行插入
TEST(LRUCacheTest, InsertWhenFull) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    cache.put(4, "four");  // 这将导致移除最早的键 1

    EXPECT_EQ(cache.get(1).value_or("not found"), "not found");  // 1 应该被移除
    EXPECT_EQ(cache.get(4).value_or("not found"), "four");  // 4 应该被插入
}

// 测试 resize 功能
TEST(LRUCacheTest, Resize) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    cache.resize(2);  // 将缓存大小缩小到 2

    EXPECT_EQ(cache.size(), 2);
    EXPECT_EQ(cache.get(1).value_or("not found"), "not found");  // 1 应该被移除
}

// 测试边缘情况: 插入相同键时更新值
TEST(LRUCacheTest, UpdateValue) {
    ThreadSafeLRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(1, "uno");  // 更新键 1 的值

    EXPECT_EQ(cache.get(1).value_or("not found"),
              "uno");  // 键 1 的值应该更新为 "uno"
}

// 测试边缘情况: 多线程并发访问
void concurrentPut(ThreadSafeLRUCache<int, std::string>& cache, int key,
                   const std::string& value) {
    cache.put(key, value);
}

void concurrentGet(ThreadSafeLRUCache<int, std::string>& cache, int key) {
    cache.get(key);
}

TEST(LRUCacheTest, ConcurrentAccess) {
    ThreadSafeLRUCache<int, std::string> cache(100);

    std::thread threadPut1(concurrentPut, std::ref(cache), 1, "one");
    std::thread threadPut2(concurrentPut, std::ref(cache), 2, "two");
    std::thread threadGet1(concurrentGet, std::ref(cache), 1);
    std::thread threadGet2(concurrentGet, std::ref(cache), 2);

    threadPut1.join();
    threadPut2.join();
    threadGet1.join();
    threadGet2.join();

    EXPECT_EQ(cache.get(1).value_or("not found"), "one");
    EXPECT_EQ(cache.get(2).value_or("not found"), "two");
}
