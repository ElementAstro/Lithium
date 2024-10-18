// curl_wrapper_test.cpp

#include <curl/curl.h>
#include <gtest/gtest.h>

#include "atom/web/curl.hpp"

using namespace atom::web;

class CurlWrapperTest : public ::testing::Test {
protected:
    CurlWrapper curlWrapper;

    static size_t dummyWriteCallback(void* contents, size_t size, size_t nmemb,
                                     void* userp) {
        return size * nmemb;
    }
};

TEST_F(CurlWrapperTest, ConstructorDestructor) {
    CurlWrapper* wrapper = new CurlWrapper();
    ASSERT_NE(wrapper, nullptr);
    delete wrapper;
}

TEST_F(CurlWrapperTest, SetUrl) {
    std::string url = "http://example.com";
    curlWrapper.setUrl(url);
    // Assuming there's a way to get the URL from the internal state for
    // verification
}

TEST_F(CurlWrapperTest, SetRequestMethod) {
    std::string method = "POST";
    curlWrapper.setRequestMethod(method);
    // Assuming there's a way to get the method from the internal state for
    // verification
}

TEST_F(CurlWrapperTest, SetHeader) {
    std::string key = "Content-Type";
    std::string value = "application/json";
    curlWrapper.setHeader(key, value);
    // Assuming there's a way to get headers from the internal state for
    // verification
}

TEST_F(CurlWrapperTest, SetOnErrorCallback) {
    bool callbackInvoked = false;
    curlWrapper.setOnErrorCallback(
        [&callbackInvoked](CURLcode) { callbackInvoked = true; });
    curlWrapper.setOnErrorCallback(
        [&callbackInvoked](CURLcode code) { callbackInvoked = true; });
    // curlWrapper.onErrorCallback(CURLE_OK); // Simulate an error
    ASSERT_TRUE(callbackInvoked);
}

TEST_F(CurlWrapperTest, SetOnResponseCallback) {
    bool callbackInvoked = false;
    curlWrapper.setOnResponseCallback(
        [&callbackInvoked](const std::string&) { callbackInvoked = true; });
    curlWrapper.setOnResponseCallback([&](const std::string& response) {
        // Simulate a response
    });
    ASSERT_TRUE(callbackInvoked);
}

TEST_F(CurlWrapperTest, SetTimeout) {
    long timeout = 30;
    curlWrapper.setTimeout(timeout);
    // Assuming there's a way to get the timeout from the internal state for
    // verification
}

TEST_F(CurlWrapperTest, SetFollowLocation) {
    bool follow = true;
    curlWrapper.setFollowLocation(follow);
    // Assuming there's a way to get the follow location flag from the internal
    // state for verification
}

TEST_F(CurlWrapperTest, SetRequestBody) {
    std::string data = "request body";
    curlWrapper.setRequestBody(data);
    // Assuming there's a way to get the request body from the internal state
    // for verification
}

TEST_F(CurlWrapperTest, SetUploadFile) {
    std::string filePath = "path/to/file";
    curlWrapper.setUploadFile(filePath);
    // Assuming there's a way to get the file path from the internal state for
    // verification
}

TEST_F(CurlWrapperTest, PerformRequest) {
    curlWrapper.setUrl("http://example.com");
    curlWrapper.setOnResponseCallback(
        [](const std::string& response) { ASSERT_FALSE(response.empty()); });
    std::string response = curlWrapper.performRequest();
    ASSERT_FALSE(response.empty());
}

TEST_F(CurlWrapperTest, AsyncPerform) {
    bool callbackInvoked = false;
    curlWrapper.setUrl("http://example.com");
    curlWrapper.asyncPerform([&callbackInvoked](const std::string& response) {
        callbackInvoked = true;
        ASSERT_FALSE(response.empty());
    });
    curlWrapper.waitAll();
    ASSERT_TRUE(callbackInvoked);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}