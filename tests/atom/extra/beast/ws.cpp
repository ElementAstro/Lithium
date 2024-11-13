#include <gtest/gtest.h>

#include "atom/extra/beast/ws.hpp"

using namespace boost::asio;
using namespace boost::beast::websocket;

// Test fixture for WSClient
class WSClientTest : public ::testing::Test {
protected:
    io_context ioc;
    WSClient client{ioc};

    void SetUp() override {
        // Set up any necessary preconditions here
    }

    void TearDown() override {
        // Clean up any necessary postconditions here
    }
};

// Test constructor
TEST_F(WSClientTest, Constructor) { EXPECT_NO_THROW(WSClient client(ioc)); }

// Test setTimeout method
TEST_F(WSClientTest, SetTimeout) {
    client.setTimeout(std::chrono::seconds(10));
    // No direct way to verify, but ensure no exceptions are thrown
}

// Test setReconnectOptions method
TEST_F(WSClientTest, SetReconnectOptions) {
    client.setReconnectOptions(5, std::chrono::seconds(2));
    // No direct way to verify, but ensure no exceptions are thrown
}

// Test setPingInterval method
TEST_F(WSClientTest, SetPingInterval) {
    client.setPingInterval(std::chrono::seconds(5));
    // No direct way to verify, but ensure no exceptions are thrown
}

// Test connect method
TEST_F(WSClientTest, Connect) {
    EXPECT_NO_THROW(client.connect("echo.websocket.org", "80"));
}

// Test send method
TEST_F(WSClientTest, Send) {
    client.connect("echo.websocket.org", "80");
    EXPECT_NO_THROW(client.send("Hello, WebSocket!"));
}

// Test receive method
TEST_F(WSClientTest, Receive) {
    client.connect("echo.websocket.org", "80");
    client.send("Hello, WebSocket!");
    EXPECT_NO_THROW({
        std::string message = client.receive();
        EXPECT_FALSE(message.empty());
    });
}

// Test close method
TEST_F(WSClientTest, Close) {
    client.connect("echo.websocket.org", "80");
    EXPECT_NO_THROW(client.close());
}

// Test asyncConnect method
TEST_F(WSClientTest, AsyncConnect) {
    bool called = false;
    client.asyncConnect("echo.websocket.org", "80",
                        [&called](beast::error_code ec) {
                            EXPECT_FALSE(ec);
                            called = true;
                        });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test asyncSend method
TEST_F(WSClientTest, AsyncSend) {
    client.connect("echo.websocket.org", "80");
    bool called = false;
    client.asyncSend("Hello, WebSocket!",
                     [&called](beast::error_code ec, std::size_t) {
                         EXPECT_FALSE(ec);
                         called = true;
                     });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test asyncReceive method
TEST_F(WSClientTest, AsyncReceive) {
    client.connect("echo.websocket.org", "80");
    client.send("Hello, WebSocket!");
    bool called = false;
    client.asyncReceive(
        [&called](beast::error_code ec, const std::string& message) {
            EXPECT_FALSE(ec);
            EXPECT_FALSE(message.empty());
            called = true;
        });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test asyncClose method
TEST_F(WSClientTest, AsyncClose) {
    client.connect("echo.websocket.org", "80");
    bool called = false;
    client.asyncClose([&called](beast::error_code ec) {
        EXPECT_FALSE(ec);
        called = true;
    });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test asyncSendJson method
TEST_F(WSClientTest, AsyncSendJson) {
    client.connect("echo.websocket.org", "80");
    json jdata = {{"message", "Hello, WebSocket!"}};
    bool called = false;
    client.asyncSendJson(jdata, [&called](beast::error_code ec, std::size_t) {
        EXPECT_FALSE(ec);
        called = true;
    });
    ioc.run();
    EXPECT_TRUE(called);
}

// Test asyncReceiveJson method
TEST_F(WSClientTest, AsyncReceiveJson) {
    client.connect("echo.websocket.org", "80");
    client.send("Hello, WebSocket!");
    bool called = false;
    client.asyncReceiveJson([&called](beast::error_code ec, const json& jdata) {
        EXPECT_FALSE(ec);
        EXPECT_FALSE(jdata.empty());
        called = true;
    });
    ioc.run();
    EXPECT_TRUE(called);
}
