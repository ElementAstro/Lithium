// TestGlobalSharedPtrManager.cpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <unordered_map>
#include <any>
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include <memory>
#include <thread>

class GlobalSharedPtrManager
{
private:
    std::unordered_map<std::string, std::any> sharedPtrMap;
    std::shared_mutex mtx;

public:
    static GlobalSharedPtrManager &getInstance()
    {
        static GlobalSharedPtrManager instance;
        return instance;
    }

    template <typename T>
    std::shared_ptr<T> getSharedPtr(const std::string &key)
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto it = sharedPtrMap.find(key);
        if (it != sharedPtrMap.end())
        {
            try
            {
                return std::any_cast<std::shared_ptr<T>>(it->second);
            }
            catch (const std::bad_any_cast &)
            {
                return nullptr; // 类型转换失败，返回空指针
            }
        }

        return nullptr;
    }

    template <typename T>
    void addSharedPtr(const std::string &key, std::shared_ptr<T> sharedPtr)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        sharedPtrMap[key] = sharedPtr;
    }

    void removeSharedPtr(const std::string &key)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        sharedPtrMap.erase(key);
    }
};

class MockObject
{
public:
    MOCK_METHOD(void, SomeMethod, ());
};

class GlobalSharedPtrManagerTest : public ::testing::Test
{
protected:
    GlobalSharedPtrManager manager;

    void SetUp() override
    {
        // Setup code
    }

    void TearDown() override
    {
        // Teardown code
    }
};

TEST_F(GlobalSharedPtrManagerTest, GetSharedPtr_ReturnsNullPtrIfKeyNotFound)
{
    std::shared_ptr<MockObject> mockObj = std::make_shared<MockObject>();

    manager.addSharedPtr("key", mockObj);

    std::shared_ptr<MockObject> result = manager.getSharedPtr<MockObject>("invalidKey");

    EXPECT_EQ(result, nullptr);
}

TEST_F(GlobalSharedPtrManagerTest, GetSharedPtr_ReturnsSharedPtrIfKeyFound)
{
    std::shared_ptr<MockObject> mockObj = std::make_shared<MockObject>();

    manager.addSharedPtr("key", mockObj);

    std::shared_ptr<MockObject> result = manager.getSharedPtr<MockObject>("key");

    EXPECT_EQ(result, mockObj);
}

TEST_F(GlobalSharedPtrManagerTest, AddSharedPtr_AddsKeyToMap)
{
    std::shared_ptr<MockObject> mockObj = std::make_shared<MockObject>();

    manager.addSharedPtr("key", mockObj);

    std::shared_ptr<MockObject> result = manager.getSharedPtr<MockObject>("key");

    EXPECT_EQ(result, mockObj);
}

TEST_F(GlobalSharedPtrManagerTest, RemoveSharedPtr_RemovesKeyFromMap)
{
    std::shared_ptr<MockObject> mockObj = std::make_shared<MockObject>();

    manager.addSharedPtr("key", mockObj);
    manager.removeSharedPtr("key");

    std::shared_ptr<MockObject> result = manager.getSharedPtr<MockObject>("key");

    EXPECT_EQ(result, nullptr);
}

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
