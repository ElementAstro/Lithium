#include <gtest/gtest.h>

#include "atom/extra/beast/http.hpp"

using namespace boost::asio;
using namespace boost::beast::http;

// Test fixture for HttpClient
class HttpClientTest : public ::testing::Test {
protected:
    io_context ioc;
    HttpClient client{ioc};

    void SetUp() override {
        // Set up any necessary preconditions here
    }

    void TearDown() override {
        // Clean up any necessary postconditions here
    }
};

// Test constructor
TEST_F(HttpClientTest, Constructor) { EXPECT_NO_THROW(HttpClient client(ioc)); }

// Test setDefaultHeader method
TEST_F(HttpClientTest, SetDefaultHeader) {
    client.setDefaultHeader("User-Agent", "TestAgent");
    // No direct way to verify, but ensure no exceptions are thrown
}

// Test setTimeout method
TEST_F(HttpClientTest, SetTimeout) {
    client.setTimeout(std::chrono::seconds(10));
    // No direct way to verify, but ensure no exceptions are thrown
}

// Test synchronous request method
TEST_F(HttpClientTest, Request) {
    EXPECT_NO_THROW({
        auto response =
            client.request(http::verb::get, "example.com", "80", "/");
        EXPECT_EQ(response.result(), status::ok);
    });
}

// Test asynchronous request method
TEST_F(HttpClientTest, AsyncRequest) {
    bool called = false;
    client.asyncRequest(
        http::verb::get, "example.com", "80", "/",
        [&called](beast::error_code ec, response<string_body> res) {
            EXPECT_FALSE(ec);
            EXPECT_EQ(res.result(), status::ok);
            called = true;
        });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test jsonRequest method
TEST_F(HttpClientTest, JsonRequest) {
    EXPECT_NO_THROW({
        auto response =
            client.jsonRequest(http::verb::get, "example.com", "80", "/");
        EXPECT_FALSE(response.empty());
    });
}

// Test asyncJsonRequest method
TEST_F(HttpClientTest, AsyncJsonRequest) {
    bool called = false;
    client.asyncJsonRequest(http::verb::get, "example.com", "80", "/",
                            [&called](beast::error_code ec, json res) {
                                EXPECT_FALSE(ec);
                                EXPECT_FALSE(res.empty());
                                called = true;
                            });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test uploadFile method
TEST_F(HttpClientTest, UploadFile) {
    EXPECT_NO_THROW({
        auto response =
            client.uploadFile("example.com", "80", "/upload", "test.txt");
        EXPECT_EQ(response.result(), status::ok);
    });
}

// Test downloadFile method
TEST_F(HttpClientTest, DownloadFile) {
    EXPECT_NO_THROW({
        client.downloadFile("example.com", "80", "/download", "downloaded.txt");
        // Verify file exists
        std::ifstream file("downloaded.txt");
        EXPECT_TRUE(file.good());
    });
}

// Test requestWithRetry method
TEST_F(HttpClientTest, RequestWithRetry) {
    EXPECT_NO_THROW({
        auto response =
            client.requestWithRetry(http::verb::get, "example.com", "80", "/");
        EXPECT_EQ(response.result(), status::ok);
    });
}

// Test batchRequest method
TEST_F(HttpClientTest, BatchRequest) {
    std::vector<std::tuple<http::verb, std::string, std::string, std::string>>
        requests = {{http::verb::get, "example.com", "80", "/"},
                    {http::verb::get, "example.com", "80", "/test"}};
    EXPECT_NO_THROW({
        auto responses = client.batchRequest(requests);
        EXPECT_EQ(responses.size(), 2);
        EXPECT_EQ(responses[0].result(), status::ok);
        EXPECT_EQ(responses[1].result(), status::ok);
    });
}

// Test asyncBatchRequest method
TEST_F(HttpClientTest, AsyncBatchRequest) {
    std::vector<std::tuple<http::verb, std::string, std::string, std::string>>
        requests = {{http::verb::get, "example.com", "80", "/"},
                    {http::verb::get, "example.com", "80", "/test"}};
    bool called = false;
    client.asyncBatchRequest(
        requests, [&called](std::vector<response<string_body>> responses) {
            EXPECT_EQ(responses.size(), 2);
            EXPECT_EQ(responses[0].result(), status::ok);
            EXPECT_EQ(responses[1].result(), status::ok);
            called = true;
        });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test runWithThreadPool method
TEST_F(HttpClientTest, RunWithThreadPool) {
    EXPECT_NO_THROW(client.runWithThreadPool(4));
}

// Test asyncDownloadFile method
TEST_F(HttpClientTest, AsyncDownloadFile) {
    bool called = false;
    client.asyncDownloadFile("example.com", "80", "/download",
                             "async_downloaded.txt",
                             [&called](beast::error_code ec, bool success) {
                                 EXPECT_FALSE(ec);
                                 EXPECT_TRUE(success);
                                 called = true;
                             });
    ioc.run();
    EXPECT_TRUE(called);
    // Verify file exists
    std::ifstream file("async_downloaded.txt");
    EXPECT_TRUE(file.good());
}
