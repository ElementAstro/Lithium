#include "atom/connection/udpclient.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <future>
#include <thread>

using namespace atom::connection;

class UdpClientTest : public ::testing::Test {
protected:
    void SetUp() override { client_ = std::make_unique<UdpClient>(); }

    void TearDown() override {
        client_->stopReceiving();
        client_.reset();
    }

    std::unique_ptr<UdpClient> client_;
};

TEST_F(UdpClientTest, Bind) { EXPECT_TRUE(client_->bind(12345)); }

TEST_F(UdpClientTest, SendReceive) {
    EXPECT_TRUE(client_->bind(12345));
    std::string message = "Hello, UDP!";
    std::vector<char> data(message.begin(), message.end());

    std::thread sender([&]() {
        UdpClient senderClient;
        EXPECT_TRUE(senderClient.send("127.0.0.1", 12345, data));
    });

    std::string remoteHost;
    int remotePort;
    auto receivedData = client_->receive(1024, remoteHost, remotePort,
                                         std::chrono::milliseconds(1000));
    EXPECT_EQ(receivedData, data);
    EXPECT_EQ(remoteHost, "127.0.0.1");

    sender.join();
}

TEST_F(UdpClientTest, AsyncReceive) {
    EXPECT_TRUE(client_->bind(12345));

    std::promise<std::vector<char>> promise;
    auto future = promise.get_future();

    client_->setOnDataReceivedCallback(
        [&](const std::vector<char>& data, const std::string& host, int port) {
            promise.set_value(data);
        });

    client_->startReceiving(1024);

    std::string message = "Hello, Async UDP!";
    std::vector<char> data(message.begin(), message.end());

    std::thread sender([&]() {
        UdpClient senderClient;
        EXPECT_TRUE(senderClient.send("127.0.0.1", 12345, data));
    });

    auto status = future.wait_for(std::chrono::seconds(5));
    ASSERT_EQ(status, std::future_status::ready);
    EXPECT_EQ(future.get(), data);

    client_->stopReceiving();
    sender.join();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
