#include "atom/memory/shared.hpp"
#include <gtest/gtest.h>
#include "atom/macro.hpp"

using namespace atom::connection;

struct TestData {
    int a;
    double b;
    char c;
} ATOM_ALIGNAS(16);

TEST(SharedMemoryTest, BasicWriteRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    TestData data{1, 2.0, 'a'};
    shm.write(data);

    SharedMemory<TestData> shmReader("/test_shm", false);
    auto readData = shmReader.read();
    EXPECT_EQ(data.a, readData.a);
    EXPECT_EQ(data.b, readData.b);
    EXPECT_EQ(data.c, readData.c);
}

TEST(SharedMemoryTest, PartialWriteRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    int newA = 42;
    shm.writePartial(newA, offsetof(TestData, a));

    SharedMemory<TestData> shmReader("/test_shm", false);
    auto readA = shmReader.readPartial<int>(offsetof(TestData, a));
    EXPECT_EQ(newA, readA);
}

TEST(SharedMemoryTest, SpanWriteRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    TestData writeData{1, 2.0, 'a'};
    std::span<const std::byte> writeSpan(
        reinterpret_cast<const std::byte*>(&writeData), sizeof(writeData));
    shm.writeSpan(writeSpan);

    SharedMemory<TestData> shmReader("/test_shm", false);
    TestData readData;
    std::span<std::byte> readSpan(reinterpret_cast<std::byte*>(&readData),
                                  sizeof(readData));
    ATOM_UNUSED_RESULT(shmReader.readSpan(readSpan));

    EXPECT_EQ(readData.a, 1);
    EXPECT_EQ(readData.b, 2.0);
    EXPECT_EQ(readData.c, 'a');
}

TEST(SharedMemoryTest, TryRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    TestData data{1, 2.0, 'a'};
    shm.write(data);

    SharedMemory<TestData> shmReader("/test_shm", false);
    auto readData = shmReader.tryRead();
    ASSERT_TRUE(readData.has_value());
    EXPECT_EQ(readData->a, data.a);
    EXPECT_EQ(readData->b, data.b);
    EXPECT_EQ(readData->c, data.c);
}
