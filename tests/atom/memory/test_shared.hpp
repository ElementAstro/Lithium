// test_shared.hpp
#ifndef ATOM_MEMORY_TEST_SHARED_HPP
#define ATOM_MEMORY_TEST_SHARED_HPP

#include "atom/memory/shared.hpp"
#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <thread>
#include <vector>

using namespace atom::connection;

// Sample trivially copyable struct for testing
struct alignas(16) TestData {
    int a;
    double b;
};

// Test fixture for SharedMemory
class SharedMemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        shm_name_ = "TestSharedMemory";
        if (SharedMemory<TestData>::exists(shm_name_)) {
            // Cleanup before test
#ifdef _WIN32
            HANDLE h =
                OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shm_name_.c_str());
            if (h) {
                CloseHandle(h);
            }
#else
            shm_unlink(shm_name_.c_str());
#endif
        }
    }

    void TearDown() override {
        if (SharedMemory<TestData>::exists(shm_name_)) {
#ifdef _WIN32
            HANDLE h =
                OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shm_name_.c_str());
            if (h) {
                CloseHandle(h);
            }
#else
            shm_unlink(shm_name_.c_str());
#endif
        }
    }

    std::string shm_name_;
};

TEST_F(SharedMemoryTest, ConstructorCreatesSharedMemory) {
    EXPECT_NO_THROW({ SharedMemory<TestData> shm(shm_name_, true); });

    EXPECT_TRUE(SharedMemory<TestData>::exists(shm_name_));
}

TEST_F(SharedMemoryTest, WriteAndRead) {
    SharedMemory<TestData> shm(shm_name_, true);

    const int K_MAGIC_NUMBER_A = 42;
    const double K_MAGIC_NUMBER_B = 3.14;
    TestData data = {K_MAGIC_NUMBER_A, K_MAGIC_NUMBER_B};
    shm.write(data);

    TestData readData = shm.read();
    EXPECT_EQ(readData.a, data.a);
    EXPECT_DOUBLE_EQ(readData.b, data.b);
}

TEST_F(SharedMemoryTest, ClearSharedMemory) {
    SharedMemory<TestData> shm(shm_name_, true);

    const int K_MAGIC_NUMBER_A = 42;
    const double K_MAGIC_NUMBER_B = 3.14;
    TestData data = {K_MAGIC_NUMBER_A, K_MAGIC_NUMBER_B};
    shm.write(data);

    shm.clear();

    TestData readData = shm.read();
    EXPECT_EQ(readData.a, 0);
    EXPECT_DOUBLE_EQ(readData.b, 0.0);
}

TEST_F(SharedMemoryTest, ResizeSharedMemory) {
    SharedMemory<TestData> shm(shm_name_, true);
    EXPECT_EQ(shm.getSize(), sizeof(TestData));

    shm.resize(sizeof(TestData) * 2);
    EXPECT_EQ(shm.getSize(), sizeof(TestData) * 2);
}

TEST_F(SharedMemoryTest, ExistsMethod) {
    EXPECT_FALSE(SharedMemory<TestData>::exists(shm_name_));

    SharedMemory<TestData> shm(shm_name_, true);
    EXPECT_TRUE(SharedMemory<TestData>::exists(shm_name_));
}

TEST_F(SharedMemoryTest, PartialWriteAndRead) {
    SharedMemory<TestData> shm(shm_name_, true);

    const int K_PARTIAL_A = 100;
    shm.writePartial(K_PARTIAL_A, offsetof(TestData, a));

    const double K_PARTIAL_B = 6.28;
    shm.writePartial(K_PARTIAL_B, offsetof(TestData, b));

    auto readA = shm.readPartial<int>(offsetof(TestData, a));
    auto readB = shm.readPartial<double>(offsetof(TestData, b));

    EXPECT_EQ(readA, K_PARTIAL_A);
    EXPECT_DOUBLE_EQ(readB, K_PARTIAL_B);
}

TEST_F(SharedMemoryTest, WritePartialOutOfBounds) {
    SharedMemory<TestData> shm(shm_name_, true);
    const int K_DATA = 100;
    EXPECT_THROW(
        {
            shm.writePartial(K_DATA, sizeof(TestData));  // Offset out of bounds
        },
        SharedMemoryException);
}

TEST_F(SharedMemoryTest, ReadPartialOutOfBounds) {
    SharedMemory<TestData> shm(shm_name_, true);
    EXPECT_THROW(
        {
            (void)shm.readPartial<int>(
                sizeof(TestData));  // Offset out of bounds
        },
        SharedMemoryException);
}

TEST_F(SharedMemoryTest, TryReadSuccess) {
    SharedMemory<TestData> shm(shm_name_, true);
    const int K_MAGIC_NUMBER_A = 42;
    const double K_MAGIC_NUMBER_B = 3.14;
    TestData data = {K_MAGIC_NUMBER_A, K_MAGIC_NUMBER_B};
    shm.write(data);

    auto result = shm.tryRead();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->a, data.a);
    EXPECT_DOUBLE_EQ(result->b, data.b);
}

TEST_F(SharedMemoryTest, TryReadFailure) {
    SharedMemory<TestData> shm(shm_name_, true);
    shm.clear();

    // Simulate timeout by using a very short timeout and holding the lock
    std::atomic<bool> lockAcquired{false};
    std::thread lockThread([&shm, &lockAcquired]() {
        shm.withLock(
            [&]() {
                lockAcquired = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            },
            std::chrono::milliseconds(200));
    });

    while (!lockAcquired.load()) {
        std::this_thread::yield();
    }

    auto result = shm.tryRead(std::chrono::milliseconds(10));
    EXPECT_FALSE(result.has_value());

    lockThread.join();
}

TEST_F(SharedMemoryTest, WriteAndReadSpan) {
    SharedMemory<TestData> shm(shm_name_, true);
    std::array<std::byte, sizeof(TestData)> dataBytes = {
        std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
    std::span<const std::byte> dataSpan(dataBytes);
    shm.writeSpan(dataSpan);

    std::array<std::byte, sizeof(TestData)> readBytes;
    std::span<std::byte> readSpan(readBytes);
    size_t bytesRead = shm.readSpan(readSpan);
    EXPECT_EQ(bytesRead, sizeof(TestData));
    EXPECT_EQ(std::memcmp(dataBytes.data(), readBytes.data(), sizeof(TestData)),
              0);
}

TEST_F(SharedMemoryTest, WriteSpanOutOfBounds) {
    SharedMemory<TestData> shm(shm_name_, true);
    std::vector<std::byte> data(sizeof(TestData) + 1, std::byte{0});
    std::span<const std::byte> dataSpan(data.data(), data.size());

    EXPECT_THROW({ shm.writeSpan(dataSpan); }, SharedMemoryException);
}

TEST_F(SharedMemoryTest, ReadSpanPartial) {
    SharedMemory<TestData> shm(shm_name_, true);
    const int K_MAGIC_NUMBER_A = 42;
    const double K_MAGIC_NUMBER_B = 3.14;
    TestData data = {K_MAGIC_NUMBER_A, K_MAGIC_NUMBER_B};
    shm.write(data);

    std::vector<std::byte> readBytes(sizeof(TestData) - 4, std::byte{0});
    std::span<std::byte> readSpan(readBytes.data(), readBytes.size());
    size_t bytesRead = shm.readSpan(readSpan);
    EXPECT_EQ(bytesRead, readBytes.size());
}

#endif  // ATOM_MEMORY_TEST_SHARED_HPP