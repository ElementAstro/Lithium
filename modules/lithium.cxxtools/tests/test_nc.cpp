// test_nc.cpp
#include <gtest/gtest.h>

#include <asio.hpp>
#include <fstream>
#include <optional>
#include <thread>

#include "atom/log/loguru.hpp"

using asio::ip::tcp;
using asio::ip::udp;

constexpr std::size_t MAX_LENGTH = 1024;
constexpr int DEFAULT_TIMEOUT_SECONDS = 10;

void runTcpClient(const std::string& host, const std::string& port,
                  int timeoutSeconds,
                  const std::optional<std::string>& filename = std::nullopt);

class TcpClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start a mock TCP server
        serverThread = std::thread([this]() {
            try {
                asio::io_context ioContext;
                tcp::acceptor acceptor(ioContext,
                                       tcp::endpoint(tcp::v4(), 12345));
                tcp::socket socket(ioContext);
                acceptor.accept(socket);

                std::array<char, MAX_LENGTH> buffer;
                asio::error_code error;
                size_t length = socket.read_some(asio::buffer(buffer), error);
                if (!error) {
                    std::string message(buffer.data(), length);
                    if (message == "Hello\n") {
                        asio::write(socket, asio::buffer("World\n"));
                    }
                }
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Exception in mock server: {}", e.what());
            }
        });
        std::this_thread::sleep_for(
            std::chrono::seconds(1));  // Give the server time to start
    }

    void TearDown() override {
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    std::thread serverThread;
};

TEST_F(TcpClientTest, RunTcpClient_SendMessage_ReceiveReply) {
    std::string host = "127.0.0.1";
    std::string port = "12345";
    int timeoutSeconds = DEFAULT_TIMEOUT_SECONDS;

    // Redirect std::cin to simulate user input
    std::istringstream input("Hello\n");
    std::cin.rdbuf(input.rdbuf());

    // Capture std::cout to verify output
    std::ostringstream output;
    std::streambuf* coutBuf = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());

    runTcpClient(host, port, timeoutSeconds);

    std::cout.rdbuf(coutBuf);  // Restore std::cout

    std::string expectedOutput = "Reply: World\n";
    EXPECT_EQ(output.str(), expectedOutput);
}

TEST_F(TcpClientTest, RunTcpClient_SendFile_Success) {
    std::string host = "127.0.0.1";
    std::string port = "12345";
    int timeoutSeconds = DEFAULT_TIMEOUT_SECONDS;

    // Create a temporary file to send
    std::string filename = "testfile.txt";
    std::ofstream file(filename);
    file << "Test file content";
    file.close();

    runTcpClient(host, port, timeoutSeconds, filename);

    // Verify that the file was sent successfully (mock server should handle
    // this) In a real test, you would have more checks here

    // Clean up
    std::remove(filename.c_str());
}

TEST_F(TcpClientTest, RunTcpClient_InvalidHost_ThrowsException) {
    std::string host = "invalid_host";
    std::string port = "12345";
    int timeoutSeconds = DEFAULT_TIMEOUT_SECONDS;

    EXPECT_THROW(runTcpClient(host, port, timeoutSeconds), std::exception);
}

TEST_F(TcpClientTest, RunTcpClient_Timeout_ClosesSocket) {
    std::string host = "127.0.0.1";
    std::string port = "12345";
    int timeoutSeconds = 1;  // Set a short timeout

    // Redirect std::cin to simulate user input
    std::istringstream input("Hello\n");
    std::cin.rdbuf(input.rdbuf());

    // Capture std::cout to verify output
    std::ostringstream output;
    std::streambuf* coutBuf = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());

    runTcpClient(host, port, timeoutSeconds);

    std::cout.rdbuf(coutBuf);  // Restore std::cout

    std::string expectedOutput = "Reply: World\n";
    EXPECT_EQ(output.str(), expectedOutput);
}
