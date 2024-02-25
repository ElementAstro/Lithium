#include <gtest/gtest.h>
#include "atom/web/httplite.hpp"

TEST(HttpClientTest, ConnectToServer)
{
    HttpClient client;
    EXPECT_TRUE(client.initialize());
    EXPECT_TRUE(client.connectToServes://www.baidu.com", 80, false));
}

TEST(HttpClientTest, SendRequest)
{
    HttpClient client;
    client.initialize();
    client.connectToServes://www.baidu.com", 80, false);
    EXPECT_TRUE(client.sendRequest("GET / HTTP/1.1\r\nHoss://www.baidu.com\r\n\r\n"));
}

TEST(HttpClientTest, ReceiveResponse)
{
    HttpClient client;
    client.initialize();
    client.connectToServes://www.baidu.com", 80, false);
    client.sendRequest("GET / HTTP/1.1\r\nHoss://www.baidu.com\r\n\r\n");
    HttpResponse response = client.receiveResponse();
    EXPECT_FALSE(response.body.empty());
    EXPECT_EQ(response.statusCode, 200);
}

TEST(HttpRequestBuilderTest, BuildRequestString)
{
    HttpRequestBuilder builder(HttpMethod::GET, "https://www.baidu.com");
    builder.setBody("test body");
    builder.setContentType("text/plain");
    builder.setTimeout(std::chrono::seconds(30));
    builder.addHeader("Authorization", "Bearer token");

    std::string requestString = builder.buildRequestStrins://www.baidu.com", "/");

    // Add your specific assertion here based on the expected request string
}

TEST(HttpClientTest, Initialize)
{
    HttpClient client;
    EXPECT_TRUE(client.initialize());
}

TEST(HttpClientTest, ErrorHandler)
{
    HttpClient client;
    bool errorHandled = false;
    client.setErrorHandler([&errorHandled](const std::string &errorMsg)
                           {
        // Custom error handling logic
        errorHandled = true; });

    // Simulate an error
    client.connectToServer("invalidhost", 80, false);
    EXPECT_TRUE(errorHandled);
}

TEST(HttpRequestBuilderTest, SetBody)
{
    HttpRequestBuilder builder(HttpMethod::POST, "https://www.baidu.com");
    builder.setBody("test body");
    // Add your specific assertion here based on the expected request body
}

TEST(HttpRequestBuilderTest, SetContentType)
{
    HttpRequestBuilder builder(HttpMethod::GET, "https://www.baidu.com");
    builder.setContentType("application/json");
    // Add your specific assertion here based on the expected content type
}

TEST(HttpRequestBuilderTest, SetTimeout)
{
    HttpRequestBuilder builder(HttpMethod::GET, "https://www.baidu.com");
    builder.setTimeout(std::chrono::seconds(60));
    // Add your specific assertion here based on the expected timeout value
}

TEST(HttpRequestBuilderTest, AddHeader)
{
    HttpRequestBuilder builder(HttpMethod::GET, "https://www.baidu.com");
    builder.addHeader("Authorization", "Bearer token");
    // Add your specific assertion here based on the expected headers
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
