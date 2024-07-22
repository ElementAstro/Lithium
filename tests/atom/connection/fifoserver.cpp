#include "atom/connection/fifoserver.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <future>
#include <thread>

using namespace atom::connection;

class FIFOServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        fifo_path_ = "/tmp/test_fifo";
        server_ = std::make_unique<FIFOServer>(fifo_path_);
    }

    void TearDown() override {
        server_->stop();
        server_.reset();
        std::filesystem::remove(fifo_path_);
    }

    std::string fifo_path_;
    std::unique_ptr<FIFOServer> server_;
};

TEST_F(FIFOServerTest, StartAndStop) {
    ASSERT_FALSE(server_->isRunning());
    server_->start();
    ASSERT_TRUE(server_->isRunning());
    server_->stop();
    ASSERT_FALSE(server_->isRunning());
}

TEST_F(FIFOServerTest, SendMessage) {
    server_->start();
    ASSERT_TRUE(server_->isRunning());

    std::string message = "Hello, FIFO!";
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();

    std::thread reader_thread([&] {
        int fd = open(fifo_path_.c_str(), O_RDONLY);
        ASSERT_NE(fd, -1);

        char buffer[1024];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        ASSERT_GT(bytes_read, 0);

        promise.set_value(std::string(buffer, bytes_read));
        close(fd);
    });

    server_->sendMessage(message);

    ASSERT_EQ(future.wait_for(std::chrono::seconds(5)),
              std::future_status::ready);
    ASSERT_EQ(future.get(), message);

    reader_thread.join();
}
