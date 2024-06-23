#include "atom/utils/to_byte.hpp"
#include <gtest/gtest.h>

TEST(SerializeTest, SerializeInt) {
    int data = 123;
    std::vector<uint8_t> bytes = atom::utils::serialize(data);

    EXPECT_EQ(bytes.size(), sizeof(int));
    int deserializedData;
    std::memcpy(&deserializedData, bytes.data(), sizeof(int));
    EXPECT_EQ(deserializedData, data);
}

TEST(SerializeTest, SerializeString) {
    std::string data = "Hello, World!";
    std::vector<uint8_t> bytes = atom::utils::serialize(data);

    EXPECT_EQ(bytes.size(), sizeof(size_t) + data.size());
    size_t size;
    std::memcpy(&size, bytes.data(), sizeof(size_t));
    EXPECT_EQ(size, data.size());
    std::string deserializedData(
        reinterpret_cast<const char*>(bytes.data() + sizeof(size)), size);
    EXPECT_EQ(deserializedData, data);
}

TEST(SerializeTest, SerializeVector) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<uint8_t> bytes = atom::utils::serialize(data);

    EXPECT_EQ(bytes.size(), sizeof(size_t) + data.size() * sizeof(int));
    size_t size;
    std::memcpy(&size, bytes.data(), sizeof(size_t));
    EXPECT_EQ(size, data.size());
    std::vector<int> deserializedData;
    deserializedData.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        int item;
        std::memcpy(&item, bytes.data() + sizeof(size) + i * sizeof(int),
                    sizeof(int));
        deserializedData.push_back(item);
    }
    EXPECT_EQ(deserializedData, data);
}
