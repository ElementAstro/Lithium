#ifndef ATOM_MEMORY_TEST_RING_BUFFER_HPP
#define ATOM_MEMORY_TEST_RING_BUFFER_HPP

#include <gtest/gtest.h>
#include <vector>

#include "atom/memory/ring.hpp"

using namespace atom::memory;

class RingBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(RingBufferTest, Constructor) {
    EXPECT_THROW(RingBuffer<int> buffer(0), std::invalid_argument);
    RingBuffer<int> buffer(10);
    EXPECT_EQ(buffer.capacity(), 10);
    EXPECT_EQ(buffer.size(), 0);
}

TEST_F(RingBufferTest, PushAndPop) {
    RingBuffer<int> buffer(3);
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));
    EXPECT_FALSE(buffer.push(4));  // Buffer should be full

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), std::nullopt);  // Buffer should be empty
}

TEST_F(RingBufferTest, PushOverwrite) {
    RingBuffer<int> buffer(3);
    buffer.pushOverwrite(1);
    buffer.pushOverwrite(2);
    buffer.pushOverwrite(3);
    buffer.pushOverwrite(4);  // Should overwrite the oldest element

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), 4);
}

TEST_F(RingBufferTest, FullAndEmpty) {
    RingBuffer<int> buffer(2);
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());

    buffer.push(1);
    buffer.push(2);
    EXPECT_FALSE(buffer.empty());
    EXPECT_TRUE(buffer.full());

    buffer.pop();
    EXPECT_FALSE(buffer.full());
    EXPECT_FALSE(buffer.empty());

    buffer.pop();
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
}

TEST_F(RingBufferTest, FrontAndBack) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    EXPECT_EQ(buffer.front(), 1);
    EXPECT_EQ(buffer.back(), 3);

    buffer.pop();
    EXPECT_EQ(buffer.front(), 2);
    EXPECT_EQ(buffer.back(), 3);
}

TEST_F(RingBufferTest, Contains) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    EXPECT_TRUE(buffer.contains(1));
    EXPECT_TRUE(buffer.contains(2));
    EXPECT_TRUE(buffer.contains(3));
    EXPECT_FALSE(buffer.contains(4));
}

TEST_F(RingBufferTest, View) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    auto view = buffer.view();
    EXPECT_EQ(view.size(), 3);
    EXPECT_EQ(view[0], 1);
    EXPECT_EQ(view[1], 2);
    EXPECT_EQ(view[2], 3);
}

TEST_F(RingBufferTest, Iterator) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    std::vector<int> elements;
    for (const auto& item : buffer) {
        elements.push_back(item);
    }

    EXPECT_EQ(elements.size(), 3);
    EXPECT_EQ(elements[0], 1);
    EXPECT_EQ(elements[1], 2);
    EXPECT_EQ(elements[2], 3);
}

TEST_F(RingBufferTest, Resize) {
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

    EXPECT_THROW(
        buffer.resize(2),
        std::runtime_error);  // Cannot resize to smaller than current size
}

TEST_F(RingBufferTest, At) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    EXPECT_EQ(buffer.at(0), 1);
    EXPECT_EQ(buffer.at(1), 2);
    EXPECT_EQ(buffer.at(2), 3);
    EXPECT_EQ(buffer.at(3), std::nullopt);  // Out of bounds
}

TEST_F(RingBufferTest, ForEach) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    buffer.forEach([](int& item) { item *= 2; });

    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 4);
    EXPECT_EQ(buffer.pop(), 6);
}

TEST_F(RingBufferTest, RemoveIf) {
    RingBuffer<int> buffer(5);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    buffer.push(5);

    buffer.removeIf([](int item) {
        return item % 2 == 0;  // Remove even numbers
    });

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), 5);
}

TEST_F(RingBufferTest, Rotate) {
    RingBuffer<int> buffer(5);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    buffer.push(5);

    buffer.rotate(2);  // Rotate left by 2
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), 4);
    EXPECT_EQ(buffer.pop(), 5);
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);

    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    buffer.push(5);

    buffer.rotate(-2);  // Rotate right by 2
    EXPECT_EQ(buffer.pop(), 4);
    EXPECT_EQ(buffer.pop(), 5);
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
}

#endif  // ATOM_MEMORY_TEST_RING_BUFFER_HPP
