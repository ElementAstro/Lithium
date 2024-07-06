#include <gtest/gtest.h>
#include "atom/memory/shared.hpp"

TEST(SharedMemoryTest, WriteReadTest) {
    std::string name = "test_shared_memory";
    atom::connection::SharedMemory<int> sharedMemory(name);

    // Write data to shared memory
    int dataToWrite = 42;
    sharedMemory.write(dataToWrite);

    // Read data from shared memory
    int dataRead = sharedMemory.read();
    EXPECT_EQ(dataToWrite, dataRead);
}

TEST(SharedMemoryTest, ClearTest) {
    std::string name = "test_shared_memory";
    atom::connection::SharedMemory<int> sharedMemory(name);

    // Write data to shared memory
    int dataToWrite = 42;
    sharedMemory.write(dataToWrite);

    // Clear shared memory
    sharedMemory.clear();

    // Read data from shared memory after clear
    int dataRead = sharedMemory.read();
    EXPECT_EQ(0, dataRead);  // Expecting cleared value
}

TEST(SharedMemoryTest, PartialWriteReadTest) {
    std::string name = "test_shared_memory";
    atom::connection::SharedMemory<char[10]> sharedMemory(name);

    // Write partial data to shared memory
    std::string dataToWrite = "Hello";
    sharedMemory.writePartial(dataToWrite, 0);

    // Read partial data from shared memory
    std::string dataRead = sharedMemory.readPartial(0);
    EXPECT_EQ(dataToWrite, dataRead);
}

TEST(SharedMemoryTest, TryReadTest) {
    std::string name = "test_shared_memory";
    atom::connection::SharedMemory<int> sharedMemory(name);

    // Try to read data from empty shared memory
    std::optional<int> dataRead = sharedMemory.tryRead();
    EXPECT_FALSE(dataRead.has_value());

    // Write data to shared memory
    int dataToWrite = 42;
    sharedMemory.write(dataToWrite);

    // Try to read data from shared memory
    dataRead = sharedMemory.tryRead();
    EXPECT_TRUE(dataRead.has_value());
    EXPECT_EQ(dataToWrite, *dataRead);
}

TEST(SharedMemoryTest, WriteSpanReadSpanTest) {
    std::string name = "test_shared_memory";
    atom::connection::SharedMemory<char[10]> sharedMemory(name);

    // Write span of data to shared memory
    std::string dataToWrite = "Hello World";
    sharedMemory.writeSpan({dataToWrite.data(), dataToWrite.size()});

    // Read span of data from shared memory
    std::string dataRead;
    sharedMemory.readSpan({dataRead.data(), dataRead.size()});
    EXPECT_EQ(dataToWrite, dataRead);
}