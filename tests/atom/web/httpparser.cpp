/*
 * test_httpparser.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/web/httpparser.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace atom::web;

// Test fixture class for setting up and tearing down parser tests
class HttpHeaderParserTest : public ::testing::Test {
protected:
    HttpHeaderParser parser;

    void SetUp() override {
        // Set up test environment, if needed
    }

    void TearDown() override {
        // Clean up after test, if needed
    }
};

// Test for parsing simple headers
TEST_F(HttpHeaderParserTest, ParseHeaders_Simple) {
    std::string rawHeaders = "Host: example.com\nUser-Agent: test-agent\n";
    parser.parseHeaders(rawHeaders);

    auto host = parser.getHeaderValues("Host");
    auto userAgent = parser.getHeaderValues("User-Agent");

    ASSERT_TRUE(host.has_value());
    ASSERT_EQ(host->size(), 1);
    EXPECT_EQ(host->at(0), "example.com");

    ASSERT_TRUE(userAgent.has_value());
    ASSERT_EQ(userAgent->size(), 1);
    EXPECT_EQ(userAgent->at(0), "test-agent");
}

// Test for setting a single header value
TEST_F(HttpHeaderParserTest, SetHeaderValue) {
    parser.setHeaderValue("Content-Type", "text/html");

    auto contentType = parser.getHeaderValues("Content-Type");

    ASSERT_TRUE(contentType.has_value());
    ASSERT_EQ(contentType->size(), 1);
    EXPECT_EQ(contentType->at(0), "text/html");
}

// Test for adding multiple values to a header
TEST_F(HttpHeaderParserTest, AddHeaderValue) {
    parser.setHeaderValue("Set-Cookie", "cookie1=value1");
    parser.addHeaderValue("Set-Cookie", "cookie2=value2");

    auto cookies = parser.getHeaderValues("Set-Cookie");

    ASSERT_TRUE(cookies.has_value());
    ASSERT_EQ(cookies->size(), 2);
    EXPECT_EQ(cookies->at(0), "cookie1=value1");
    EXPECT_EQ(cookies->at(1), "cookie2=value2");
}

// Test for checking header existence
TEST_F(HttpHeaderParserTest, HasHeader) {
    parser.setHeaderValue("Authorization", "Bearer token");

    EXPECT_TRUE(parser.hasHeader("Authorization"));
    EXPECT_FALSE(parser.hasHeader("Non-Existent-Header"));
}

// Test for removing a header
TEST_F(HttpHeaderParserTest, RemoveHeader) {
    parser.setHeaderValue("Connection", "keep-alive");
    EXPECT_TRUE(parser.hasHeader("Connection"));

    parser.removeHeader("Connection");
    EXPECT_FALSE(parser.hasHeader("Connection"));
}

// Test for clearing all headers
TEST_F(HttpHeaderParserTest, ClearHeaders) {
    parser.setHeaderValue("Accept", "text/html");
    EXPECT_TRUE(parser.hasHeader("Accept"));

    parser.clearHeaders();
    EXPECT_FALSE(parser.hasHeader("Accept"));
}

// Test for setting multiple headers at once
TEST_F(HttpHeaderParserTest, SetHeaders) {
    std::map<std::string, std::vector<std::string>> headers = {
        {"Accept-Encoding", {"gzip", "deflate"}},
        {"User-Agent", {"gtest-agent"}}};
    parser.setHeaders(headers);

    auto encoding = parser.getHeaderValues("Accept-Encoding");
    auto userAgent = parser.getHeaderValues("User-Agent");

    ASSERT_TRUE(encoding.has_value());
    ASSERT_EQ(encoding->size(), 2);
    EXPECT_EQ(encoding->at(0), "gzip");
    EXPECT_EQ(encoding->at(1), "deflate");

    ASSERT_TRUE(userAgent.has_value());
    ASSERT_EQ(userAgent->size(), 1);
    EXPECT_EQ(userAgent->at(0), "gtest-agent");
}

// Test for getting all headers
TEST_F(HttpHeaderParserTest, GetAllHeaders) {
    parser.setHeaderValue("Accept", "text/html");
    parser.setHeaderValue("Content-Type", "application/json");

    auto allHeaders = parser.getAllHeaders();

    ASSERT_EQ(allHeaders.size(), 2);
    EXPECT_EQ(allHeaders["Accept"].at(0), "text/html");
    EXPECT_EQ(allHeaders["Content-Type"].at(0), "application/json");
}
