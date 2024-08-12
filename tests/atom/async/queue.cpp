#include "atom/async/queue.hpp"
#include <gtest/gtest.h>
#include <thread>

TEST(ThreadSafeQueueTest, PutAndTake) {
    atom::async::ThreadSafeQueue<int> queue;

    // Put elements into the queue
    queue.put(1);
    queue.put(2);
    queue.put(3);

    // Take elements from the queue
    EXPECT_EQ(queue.take(), 1);
    EXPECT_EQ(queue.take(), 2);
    EXPECT_EQ(queue.take(), 3);
    EXPECT_FALSE(queue.take());  // Queue should be empty now
}

TEST(ThreadSafeQueueTest, Destroy) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    auto destroyedQueue = queue.destroy();
    EXPECT_EQ(destroyedQueue.size(), 3);
    EXPECT_TRUE(queue.empty());  // Original queue should be empty now
}

TEST(ThreadSafeQueueTest, Size) {
    atom::async::ThreadSafeQueue<int> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.put(1);
    queue.put(2);
    queue.put(3);

    EXPECT_EQ(queue.size(), 3);
}

TEST(ThreadSafeQueueTest, Empty) {
    atom::async::ThreadSafeQueue<int> queue;
    EXPECT_TRUE(queue.empty());

    queue.put(1);
    EXPECT_FALSE(queue.empty());
}

TEST(ThreadSafeQueueTest, FrontAndBack) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    EXPECT_EQ(queue.front(), 1);
    EXPECT_EQ(queue.back(), 3);
}

TEST(ThreadSafeQueueTest, Emplace) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.emplace(1);
    queue.emplace(2);
    queue.emplace(3);

    EXPECT_EQ(queue.take(), 1);
    EXPECT_EQ(queue.take(), 2);
    EXPECT_EQ(queue.take(), 3);
}

TEST(ThreadSafeQueueTest, WaitAndTake) {
    atom::async::ThreadSafeQueue<int> queue;
    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.put(1);
    });

    EXPECT_EQ(queue.waitFor([](int x) { return x == 1; }), 1);
    producer.join();
}

TEST(ThreadSafeQueueTest, WaitUntilEmpty) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    queue.take();
    queue.take();

    queue.waitUntilEmpty();
    EXPECT_TRUE(queue.empty());
}

TEST(ThreadSafeQueueTest, ExtractIf) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);
    queue.put(4);
    queue.put(5);

    auto extracted = queue.extractIf([](int x) { return x % 2 == 0; });

    EXPECT_EQ(extracted.size(), 2);
    EXPECT_TRUE(std::all_of(extracted.begin(), extracted.end(),
                            [](int x) { return x % 2 == 0; }));

    EXPECT_EQ(queue.size(), 3);
    EXPECT_TRUE(std::all_of(queue.toVector().begin(), queue.toVector().end(),
                            [](int x) { return x % 2 != 0; }));
}

TEST(ThreadSafeQueueTest, Sort) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(3);
    queue.put(1);
    queue.put(2);

    queue.sort([](int a, int b) { return a < b; });

    EXPECT_EQ(queue.take(), 1);
    EXPECT_EQ(queue.take(), 2);
    EXPECT_EQ(queue.take(), 3);
}

TEST(ThreadSafeQueueTest, Transform) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    auto transformedQueue =
        queue.transform<double>([](int x) -> double { return x * 2; });

    EXPECT_EQ(transformedQueue->take(), 2);
    EXPECT_EQ(transformedQueue->take(), 4);
    EXPECT_EQ(transformedQueue->take(), 6);
}

TEST(ThreadSafeQueueTest, GroupBy) {
    auto intQueue = std::make_shared<atom::async::ThreadSafeQueue<int>>();

    // 添加一些元素
    for (int i = 0; i <= 4; ++i) {
        intQueue->put(i);
    }
    auto groupedQueues = intQueue->groupBy<std::string>(
        [](const int& x) { return (x % 2 == 0) ? "even" : "odd"; });

    EXPECT_EQ(groupedQueues.size(), 4);

    EXPECT_EQ(groupedQueues[0].get(),
              (std::vector{"even", "odd", "even", "odd", "even"}));
}

TEST(ThreadSafeQueueTest, ToVector) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    auto vector = queue.toVector();

    EXPECT_EQ(vector.size(), 3);
    EXPECT_EQ(vector, std::vector<int>({1, 2, 3}));
}

TEST(ThreadSafeQueueTest, ForEach) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);
    queue.put(2);
    queue.put(3);

    std::vector<int> results;
    queue.forEach([&results](int x) { results.push_back(x * 2); });

    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results, std::vector<int>({2, 4, 6}));
}

TEST(ThreadSafeQueueTest, TryTake) {
    atom::async::ThreadSafeQueue<int> queue;
    queue.put(1);

    EXPECT_EQ(queue.tryTake(), 1);
    EXPECT_FALSE(queue.tryTake());  // Queue should be empty now
}

TEST(ThreadSafeQueueTest, TakeFor) {
    atom::async::ThreadSafeQueue<int> queue;

    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.put(1);
    });

    EXPECT_EQ(queue.takeFor(std::chrono::milliseconds(200)), 1);
    producer.join();
}

TEST(ThreadSafeQueueTest, TakeUntil) {
    atom::async::ThreadSafeQueue<int> queue;

    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.put(1);
    });

    auto timeoutTime =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
    EXPECT_EQ(queue.takeUntil(timeoutTime), 1);
    producer.join();
}
