#include "atom/memory/ring.hpp"
#include <gtest/gtest.h>


// 测试构造函数
TEST(RingBufferTest, Constructor) {
    RingBuffer<int> buffer(5);
    EXPECT_EQ(buffer.capacity(), 5);
    EXPECT_EQ(buffer.size(), 0);
}

// 测试push函数
TEST(RingBufferTest, Push) {
    RingBuffer<int> buffer(3);
    EXPECT_TRUE(buffer.push(1));
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_TRUE(buffer.push(2));
    EXPECT_EQ(buffer.size(), 2);
    EXPECT_FALSE(buffer.push(3));
    EXPECT_EQ(buffer.size(), 2);
}

// 测试pushOverwrite函数
TEST(RingBufferTest, PushOverwrite) {
    RingBuffer<int> buffer(3);
    buffer.pushOverwrite(1);
    buffer.pushOverwrite(2);
    buffer.pushOverwrite(3);
    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.at(0), 3);
    buffer.pushOverwrite(4);
    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.at(0), 4);
}

// 测试pop函数
TEST(RingBufferTest, Pop) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), std::nullopt);
}

// 测试full函数
TEST(RingBufferTest, Full) {
    RingBuffer<int> buffer(3);
    EXPECT_FALSE(buffer.full());
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_TRUE(buffer.full());
}

// 测试empty函数
TEST(RingBufferTest, Empty) {
    RingBuffer<int> buffer(3);
    EXPECT_TRUE(buffer.empty());
    buffer.push(1);
    EXPECT_FALSE(buffer.empty());
    buffer.pop();
    EXPECT_TRUE(buffer.empty());
}

// 测试size函数
TEST(RingBufferTest, Size) {
    RingBuffer<int> buffer(3);
    EXPECT_EQ(buffer.size(), 0);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.size(), 3);
    buffer.pop();
    buffer.pop();
    EXPECT_EQ(buffer.size(), 1);
}

// 测试capacity函数
TEST(RingBufferTest, Capacity) {
    RingBuffer<int> buffer(5);
    EXPECT_EQ(buffer.capacity(), 5);
}

// 测试clear函数
TEST(RingBufferTest, Clear) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.clear();
    EXPECT_EQ(buffer.size(), 0);
}

// 测试front函数
TEST(RingBufferTest, Front) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.front(), 1);
}

// 测试back函数
TEST(RingBufferTest, Back) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.back(), 3);
}

// 测试contains函数
TEST(RingBufferTest, Contains) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_TRUE(buffer.contains(2));
    EXPECT_FALSE(buffer.contains(4));
}

// 测试view函数
TEST(RingBufferTest, View) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    auto view = buffer.view();
    EXPECT_EQ(view, std::vector<int>({1, 2, 3}));
}

// 测试begin和end函数
TEST(RingBufferTest, Iterator) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    auto it = buffer.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(it, buffer.end());
}

// 测试resize函数
TEST(RingBufferTest, Resize) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.resize(5);
    EXPECT_EQ(buffer.capacity(), 5);
    EXPECT_EQ(buffer.size(), 3);
    buffer.push(4);
    buffer.push(5);
    EXPECT_EQ(buffer.size(), 5);
    buffer.pop();
    buffer.pop();
    EXPECT_EQ(buffer.size(), 3);
}

// 测试at函数
TEST(RingBufferTest, At) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    EXPECT_EQ(buffer.at(0), 1);
    EXPECT_EQ(buffer.at(1), 2);
    EXPECT_EQ(buffer.at(2), 3);
    EXPECT_EQ(buffer.at(3), std::nullopt);
}

// 测试forEach函数
TEST(RingBufferTest, ForEach) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.forEach([](int& elem) { elem *= 2; });
    auto view = buffer.view();
    EXPECT_EQ(view, std::vector<int>({2, 4, 6}));
}

// 测试removeIf函数
TEST(RingBufferTest, RemoveIf) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.removeIf([](int elem) { return elem % 2 == 0; });
    auto view = buffer.view();
    EXPECT_EQ(view, std::vector<int>({1, 3}));
}

// 测试rotate函数
TEST(RingBufferTest, Rotate) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.rotate(1);
    auto view = buffer.view();
    EXPECT_EQ(view, std::vector<int>({2, 3, 1}));
    buffer.rotate(-1);
    view = buffer.view();
    EXPECT_EQ(view, std::vector<int>({1, 2, 3}));
}