#include "atom/memory/object.hpp"
#include <gtest/gtest.h>

class TestObject {
public:
    void reset() { value = 0; }
    int value = 42;
};

// Tests for ObjectPool class
TEST(ObjectPoolTest, AcquireAndRelease) {
    ObjectPool<TestObject> pool(2);

    auto obj1 = pool.acquire();
    EXPECT_EQ(obj1->value, 42);
    obj1->value = 10;

    auto obj2 = pool.acquire();
    EXPECT_EQ(obj2->value, 42);
    obj2->value = 20;

    pool.release(std::move(obj1));
    pool.release(std::move(obj2));

    auto obj3 = pool.acquire();
    EXPECT_EQ(obj3->value, 0);  // The value should be reset
    auto obj4 = pool.acquire();
    EXPECT_EQ(obj4->value, 0);  // The value should be reset
}

TEST(ObjectPoolTest, MaxSize) {
    ObjectPool<TestObject> pool(2);

    auto obj1 = pool.acquire();
    auto obj2 = pool.acquire();

    EXPECT_THROW(pool.acquire(),
                 std::runtime_error);  // No more objects available
}

TEST(ObjectPoolTest, Prefill) {
    ObjectPool<TestObject> pool(3);
    pool.prefill(2);

    EXPECT_EQ(pool.available(), 3);

    auto obj1 = pool.acquire();
    auto obj2 = pool.acquire();

    EXPECT_EQ(pool.available(), 1);
}

TEST(ObjectPoolTest, Available) {
    ObjectPool<TestObject> pool(3);

    EXPECT_EQ(pool.available(), 3);

    auto obj1 = pool.acquire();
    EXPECT_EQ(pool.available(), 2);

    pool.release(std::move(obj1));
    EXPECT_EQ(pool.available(), 3);
}

TEST(ObjectPoolTest, Size) {
    ObjectPool<TestObject> pool(3);

    EXPECT_EQ(pool.size(), 0);

    auto obj1 = pool.acquire();
    EXPECT_EQ(pool.size(), 1);

    auto obj2 = pool.acquire();
    EXPECT_EQ(pool.size(), 2);

    pool.release(std::move(obj1));
    EXPECT_EQ(pool.size(), 1);
}

TEST(ObjectPoolTest, MultiThreadedAcquireRelease) {
    ObjectPool<TestObject> pool(10);
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&pool, &counter] {
            for (int j = 0; j < 20; ++j) {
                auto obj = pool.acquire();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                pool.release(std::move(obj));
                ++counter;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, 100);
    EXPECT_EQ(pool.available(), 10);
}
