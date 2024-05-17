#include <gtest/gtest.h>
#include <thread>
#include "atom/async/queue.hpp"

// 测试 ThreadSafeQueue 的基本功能
TEST(ThreadSafeQueueTest, BasicFunctionality)
{
    Atom::Async::ThreadSafeQueue<int> queue;

    // 测试 put 和 take
    queue.put(1);
    ASSERT_EQ(queue.size(), 1);
    auto value = queue.take();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value.value(), 1);
    ASSERT_EQ(queue.size(), 0);

    // 测试空队列的 take
    value = queue.take();
    ASSERT_FALSE(value.has_value());

    // 测试前后元素访问
    queue.put(2);
    queue.put(3);
    ASSERT_EQ(queue.front().value(), 2);
    ASSERT_EQ(queue.back().value(), 3);

    // 测试清空队列
    queue.clear();
    ASSERT_TRUE(queue.empty());
}

// 测试等待操作
TEST(ThreadSafeQueueTest, WaitForFunction)
{
    Atom::Async::ThreadSafeQueue<int> queue;

    // 测试 waitFor
    std::thread t([&]()
                  {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.put(1); });

    auto value = queue.waitFor([](const std::queue<int> &q)
                               { return !q.empty(); });
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value.value(), 1);

    t.join();
}

// 测试销毁队列
TEST(ThreadSafeQueueTest, DestroyFunction)
{
    Atom::Async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    auto result = queue.destroy();
    ASSERT_EQ(result.size(), 3);
    ASSERT_TRUE(queue.empty());
}

// 测试emplace功能
TEST(ThreadSafeQueueTest, EmplaceFunction)
{
    Atom::Async::ThreadSafeQueue<std::string> queue;

    queue.emplace("Hello", "World");
    ASSERT_EQ(queue.size(), 1);
    auto value = queue.take();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value.value(), "Hello");
}

// 测试等待队列为空
TEST(ThreadSafeQueueTest, WaitForEmptyFunction)
{
    Atom::Async::ThreadSafeQueue<int> queue;

    std::thread t([&]()
                  {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.put(1); });

    queue.waitUntilEmpty();

    ASSERT_TRUE(queue.empty());

    t.join();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
