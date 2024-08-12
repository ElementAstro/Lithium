#include "atom/search/cache.hpp"

#include <gtest/gtest.h>
#include <thread>

struct TestStruct {
    int id;
    std::string name;

    bool operator==(const TestStruct &other) const {
        return id == other.id && name == other.name;
    }
};

std::string serializeTestStruct(const TestStruct &ts) {
    return std::to_string(ts.id) + "," + ts.name;
}

TestStruct deserializeTestStruct(const std::string &str) {
    auto pos = str.find(',');
    int id = std::stoi(str.substr(0, pos));
    std::string name = str.substr(pos + 1);
    return {id, name};
}

json toJson(const TestStruct &ts) {
    return json{{"id", ts.id}, {"name", ts.name}};
}

TestStruct fromJson(const json &j) {
    return {j.at("id").get<int>(), j.at("name").get<std::string>()};
}

class ResourceCacheTest : public ::testing::Test {
protected:
    ResourceCache<int> intCache;
    ResourceCache<TestStruct> structCache;

    ResourceCacheTest() : intCache(3), structCache(3) {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(ResourceCacheTest, InsertAndGetInt) {
    intCache.insert("key1", 100, std::chrono::seconds(10));
    EXPECT_TRUE(intCache.contains("key1"));
    EXPECT_EQ(intCache.get("key1"), 100);
}

TEST_F(ResourceCacheTest, InsertAndGetStruct) {
    TestStruct ts{1, "test"};
    structCache.insert("key1", ts, std::chrono::seconds(10));
    EXPECT_TRUE(structCache.contains("key1"));
    EXPECT_EQ(structCache.get("key1"), ts);
}

TEST_F(ResourceCacheTest, Expiration) {
    std::cout << "Testing expiration..." << std::endl;
    intCache.insert("key1", 100, std::chrono::seconds(1));
    std::cout << "Sleeping for 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Sleeping for 1 second..." << std::endl;
    EXPECT_FALSE(intCache.contains("key1"));
}

TEST_F(ResourceCacheTest, Eviction) {
    intCache.insert("key1", 100, std::chrono::seconds(10));
    intCache.insert("key2", 200, std::chrono::seconds(10));
    intCache.insert("key3", 300, std::chrono::seconds(10));
    intCache.insert("key4", 400,
                    std::chrono::seconds(10));  // This should evict "key1"
    EXPECT_FALSE(intCache.contains("key1"));
    EXPECT_TRUE(intCache.contains("key4"));
}

TEST_F(ResourceCacheTest, AsyncGet) {
    intCache.insert("key1", 100, std::chrono::seconds(10));
    auto future = intCache.asyncGet("key1");
    EXPECT_EQ(future.get(), 100);
}

TEST_F(ResourceCacheTest, AsyncInsert) {
    auto future = intCache.asyncInsert("key1", 100, std::chrono::seconds(10));
    future.get();
    EXPECT_TRUE(intCache.contains("key1"));
}

TEST_F(ResourceCacheTest, Clear) {
    intCache.insert("key1", 100, std::chrono::seconds(10));
    intCache.clear();
    EXPECT_FALSE(intCache.contains("key1"));
}

TEST_F(ResourceCacheTest, ReadFromFile) {
    std::string filePath = "test_cache.txt";
    std::ofstream outFile(filePath);
    outFile << "key1:100\nkey2:200\n";
    outFile.close();

    intCache.readFromFile(
        filePath, [](const std::string &str) { return std::stoi(str); });
    EXPECT_TRUE(intCache.contains("key1"));
    EXPECT_TRUE(intCache.contains("key2"));
    EXPECT_EQ(intCache.get("key1"), 100);
    EXPECT_EQ(intCache.get("key2"), 200);
}

TEST_F(ResourceCacheTest, WriteToFile) {
    std::string filePath = "test_cache_write.txt";
    intCache.insert("key1", 100, std::chrono::seconds(10));
    intCache.insert("key2", 200, std::chrono::seconds(10));
    intCache.writeToFile(
        filePath, [](const int &value) { return std::to_string(value); });

    std::ifstream inFile(filePath);
    std::string content((std::istreambuf_iterator<char>(inFile)),
                        std::istreambuf_iterator<char>());
    inFile.close();

    EXPECT_TRUE(content.find("key1:100") != std::string::npos);
    EXPECT_TRUE(content.find("key2:200") != std::string::npos);
}

TEST_F(ResourceCacheTest, ReadFromJsonFile) {
    std::string filePath = "test_cache.json";
    std::ofstream outFile(filePath);
    outFile
        << R"({"key1":{"id":1,"name":"test1"},"key2":{"id":2,"name":"test2"}})";
    outFile.close();

    structCache.readFromJsonFile(filePath, fromJson);
    EXPECT_TRUE(structCache.contains("key1"));
    EXPECT_TRUE(structCache.contains("key2"));
    EXPECT_EQ(structCache.get("key1"), (TestStruct{1, "test1"}));
    EXPECT_EQ(structCache.get("key2"), (TestStruct{2, "test2"}));
}

TEST_F(ResourceCacheTest, WriteToJsonFile) {
    std::string filePath = "test_cache_write.json";
    structCache.insert("key1", {1, "test1"}, std::chrono::seconds(10));
    structCache.insert("key2", {2, "test2"}, std::chrono::seconds(10));
    structCache.writeToJsonFile(filePath, toJson);

    std::ifstream inFile(filePath);
    std::string content((std::istreambuf_iterator<char>(inFile)),
                        std::istreambuf_iterator<char>());
    inFile.close();

    EXPECT_TRUE(content.find(R"("key1":{"id":1,"name":"test1"})") !=
                std::string::npos);
    EXPECT_TRUE(content.find(R"("key2":{"id":2,"name":"test2"})") !=
                std::string::npos);
}
