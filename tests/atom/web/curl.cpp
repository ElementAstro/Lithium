#include <gtest/gtest.h>
#include "atom/web/curl.hpp"

// Test fixture for CurlWrapper class
class CurlWrapperTest : public ::testing::Test
{
protected:
    CurlWrapper curl;

    void SetUp() override
    {
        // Initialize CurlWrapper before each test
    }

    void TearDown() override
    {
        // Clean up CurlWrapper after each test
    }
};

// Test case to check if setting URL works correctly
TEST_F(CurlWrapperTest, SetUrlTest)
{
    std::string testUrl = "https://www.example.com";
    curl.setUrl(testUrl);
    EXPECT_EQ(testUrl, /* Get the URL from CurlWrapper instance and compare */);
}

// Test case to check if performing a request returns non-empty response data
TEST_F(CurlWrapperTest, PerformRequestTest)
{
    std::string testUrl = "https://www.example.com";
    curl.setUrl(testUrl);
    std::string responseData = curl.performRequest();
    EXPECT_FALSE(responseData.empty());
}

// Test case to check if setting header works correctly
TEST_F(CurlWrapperTest, SetHeaderTest)
{
    std::string testKey = "Content-Type";
    std::string testValue = "application/json";
    curl.setHeader(testKey, testValue);
    EXPECT_TRUE(/* Check if the header is set correctly in CurlWrapper instance */);
}

// Test case to check if setting request method works correctly
TEST_F(CurlWrapperTest, SetRequestMethodTest)
{
    std::string testMethod = "POST";
    curl.setRequestMethod(testMethod);
    EXPECT_EQ(testMethod, /* Get the request method from CurlWrapper instance and compare */);
}

// Test case to check if setting timeout works correctly
TEST_F(CurlWrapperTest, SetTimeoutTest)
{
    long testTimeout = 10L;
    curl.setTimeout(testTimeout);
    EXPECT_EQ(testTimeout, /* Get the timeout value from CurlWrapper instance and compare */);
}

// Test case to check if setting follow location works correctly
TEST_F(CurlWrapperTest, SetFollowLocationTest)
{
    bool testFollowLocation = true;
    curl.setFollowLocation(testFollowLocation);
    EXPECT_EQ(testFollowLocation, /* Get the follow location flag from CurlWrapper instance and compare */);
}

// Test case to check if setting request body works correctly
TEST_F(CurlWrapperTest, SetRequestBodyTest)
{
    std::string testRequestBody = "{\"name\":\"John\",\"age\":30,\"city\":\"New York\"}";
    curl.setRequestBody(testRequestBody);
    EXPECT_EQ(testRequestBody, /* Get the request body data from CurlWrapper instance and compare */);
}

// Test case to check if setting upload file works correctly
TEST_F(CurlWrapperTest, SetUploadFileTest)
{
    std::string testFilePath = "/path/to/test/file.txt";
    curl.setUploadFile(testFilePath);
    EXPECT_EQ(testFilePath, /* Get the upload file path from CurlWrapper instance and compare */);
}

// Test case to check if setting error callback works correctly
TEST_F(CurlWrapperTest, SetOnErrorCallbackTest)
{
    bool callbackCalled = false;
    curl.setOnErrorCallback([&callbackCalled](CURLcode code)
                            {
        // Set flag and handle error
        callbackCalled = true; });
    /* Trigger an error and ensure the callback was called */
    EXPECT_TRUE(callbackCalled);
}

// Test case to check if setting response callback works correctly
TEST_F(CurlWrapperTest, SetOnResponseCallbackTest)
{
    std::string responseBody = "The quick brown fox jumps over the lazy dog";
    curl.setOnResponseCallback([&responseBody](const std::string &data)
                               {
        // Handle response data
        responseBody = data; });
    /* Perform a request and ensure the response data was handled by the callback */
    EXPECT_EQ(responseBody, /* Expected response data */);
}

// Test case to check if asynchronous request works correctly
TEST_F(CurlWrapperTest, AsyncPerformTest)
{
    std::string testUrl = "https://www.example.com";
    curl.setUrl(testUrl);
    bool callbackCalled = false;
    curl.asyncPerform([&callbackCalled](const std::string &data)
                      {
        // Handle response data and set flag
        callbackCalled = true; });
    /* Wait for response and ensure the callback was called */
    curl.waitAll();
    EXPECT_TRUE(callbackCalled);
}

// Main function to run all the tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
