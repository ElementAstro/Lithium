#include "atom/memory/shared.hpp"
#include <gtest/gtest.h>


using namespace atom::connection;

struct TestData {
    int a;
    double b;
    char c;
};

TEST(SharedMemoryTest, BasicWriteRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    TestData data{1, 2.0, 'a'};
    shm.write(data);

    SharedMemory<TestData> shm_reader("/test_shm", false);
    auto read_data = shm_reader.read();
    EXPECT_EQ(data.a, read_data.a);
    EXPECT_EQ(data.b, read_data.b);
    EXPECT_EQ(data.c, read_data.c);
}

TEST(SharedMemoryTest, PartialWriteRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    int new_a = 42;
    shm.writePartial(new_a, offsetof(TestData, a));

    SharedMemory<TestData> shm_reader("/test_shm", false);
    auto read_a = shm_reader.readPartial<int>(offsetof(TestData, a));
    EXPECT_EQ(new_a, read_a);
}

TEST(SharedMemoryTest, SpanWriteRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    TestData write_data{1, 2.0, 'a'};
    std::span<const std::byte> write_span(reinterpret_cast<const std::byte*>(&write_data), sizeof(write_data));
    shm.writeSpan(write_span);

    SharedMemory<TestData> shm_reader("/test_shm", false);
    TestData read_data;
    std::span<std::byte> read_span(reinterpret_cast<std::byte*>(&read_data), sizeof(read_data));
    shm_reader.readSpan(read_span);

    EXPECT_EQ(read_data.a, 1);
    EXPECT_EQ(read_data.b, 2.0);
    EXPECT_EQ(read_data.c, 'a');
}

TEST(SharedMemoryTest, TryRead) {
    SharedMemory<TestData> shm("/test_shm", true);
    TestData data{1, 2.0, 'a'};
    shm.write(data);

    SharedMemory<TestData> shm_reader("/test_shm", false);
    auto read_data = shm_reader.tryRead();
    ASSERT_TRUE(read_data.has_value());
    EXPECT_EQ(read_data->a, data.a);
    EXPECT_EQ(read_data->b, data.b);
    EXPECT_EQ(read_data->c, data.c);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
