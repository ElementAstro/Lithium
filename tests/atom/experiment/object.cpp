#include "atom/experiment/object.hpp"
#include <gtest/gtest.h>
#include <thread>

class TestObject {
public:
    void reset() { value_ = 0; }
    int value_ = 0;
};

class ObjectPoolTest : public ::testing::Test {
protected:
    ObjectPool<TestObject> pool_{10};
};

TEST_F(ObjectPoolTest, Acquire) {
    auto obj = pool_.acquire();
    EXPECT_TRUE(obj != nullptr);
    EXPECT_EQ(obj->value_, 0);
}

TEST_F(ObjectPoolTest, Release) {
    auto obj = pool_.acquire();
    obj->value_ = 42;
    pool_.release(std::move(obj));
    obj = pool_.acquire();
    EXPECT_EQ(obj->value_, 0);
}

/*
TEST_F(ObjectPoolTest, MaxSize) {
    std::vector<std::shared_ptr<TestObject>> objects;
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool_.acquire());
    }
    EXPECT_EQ(pool_.available(), 0);
    auto obj = pool_.acquire();
    EXPECT_TRUE(obj != nullptr);
    EXPECT_EQ(pool_.size(), 10);
}
*/

TEST_F(ObjectPoolTest, ReleaseMoreThanAcquire) {
    auto obj1 = pool_.acquire();
    auto obj2 = pool_.acquire();
    pool_.release(std::move(obj1));
    pool_.release(std::move(obj2));
    auto obj3 = std::make_shared<TestObject>();
    pool_.release(std::move(obj3));
    EXPECT_EQ(pool_.size(), 2);
    EXPECT_EQ(pool_.available(), 10);
}

TEST_F(ObjectPoolTest, ConcurrentAccess) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this]() {
            auto obj = pool_.acquire();
            obj->value_ = 42;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            pool_.release(std::move(obj));
        });
    }
    for (auto &t : threads) {
        t.join();
    }
    EXPECT_EQ(pool_.size(), 10);
    EXPECT_EQ(pool_.available(), 10);
    auto obj = pool_.acquire();
    EXPECT_EQ(obj->value_, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}