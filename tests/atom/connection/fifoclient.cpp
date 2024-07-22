#include "atom/connection/fifoclient.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <future>
#include <thread>
#include "atom/connection/fifoserver.hpp"

using namespace atom::connection;

class FifoClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        fifo_path_ = "/tmp/test_fifo";
        server_ = std::make_unique<FIFOServer>(fifo_path_);
        client_ = std::make_unique<FifoClient>(fifo_path_);
        server_->start();
    }

    void TearDown() override {
        server_->stop();
        client_->close();
        server_.reset();
        client_.reset();
        std::filesystem::remove(fifo_path_);
    }

    std::string fifo_path_;
    std::unique_ptr<FIFOServer> server_;
    std::unique_ptr<FifoClient> client_;
};

TEST_F(FifoClientTest, ConnectToFifo) { ASSERT_TRUE(client_->isOpen()); }

TEST_F(FifoClientTest, WriteToFifo) {
    ASSERT_TRUE(client_->isOpen());

    std::string message = "Hello, FIFO!";
    ASSERT_TRUE(client_->write(message));
}

TEST_F(FifoClientTest, ReadFromFifo) {
    ASSERT_TRUE(client_->isOpen());

    std::string message = "Hello, FIFO!";
    server_->sendMessage(message);

    auto future = std::async(std::launch::async, [&]() {
        return client_->read(std::chrono::seconds(5));
    });

    auto status = future.wait_for(std::chrono::seconds(6));
    ASSERT_EQ(status, std::future_status::ready);

    auto result = future.get();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), message);
}

TEST_F(FifoClientTest, WriteAndReadWithTimeout) {
    ASSERT_TRUE(client_->isOpen());

    std::string message = "Hello, FIFO!";
    ASSERT_TRUE(client_->write(message, std::chrono::seconds(1)));

    auto future = std::async(std::launch::async, [&]() {
        return client_->read(std::chrono::seconds(1));
    });

    auto status = future.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(status, std::future_status::ready);

    auto result = future.get();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), message);
}

TEST_F(FifoClientTest, ReadTimeout) {
    ASSERT_TRUE(client_->isOpen());

    auto future = std::async(std::launch::async, [&]() {
        return client_->read(std::chrono::seconds(1));
    });

    auto status = future.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(status, std::future_status::ready);

    auto result = future.get();
    ASSERT_FALSE(result.has_value());
}
