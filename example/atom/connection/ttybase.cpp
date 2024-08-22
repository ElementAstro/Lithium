#include <iostream>
#include <string>
#include <vector>

#include "atom/connection/ttybase.hpp"

// Derived class implementation for demonstration purposes
class MyTtyClient : public TTYBase {
public:
    explicit MyTtyClient(std::string_view driverName) : TTYBase(driverName) {}

    // Example of connecting to a TTY device
    void exampleConnect(const std::string& device) {
        uint32_t baudRate = 9600;  // Set baud rate
        uint8_t wordSize = 8;      // 8 data bits
        uint8_t parity = 0;        // No parity
        uint8_t stopBits = 1;      // 1 stop bit

        auto response = connect(device, baudRate, wordSize, parity, stopBits);
        if (response == TTYResponse::OK) {
            std::cout << "Connected to " << device << " successfully."
                      << std::endl;
        } else {
            std::cerr << "Failed to connect: " << getErrorMessage(response)
                      << std::endl;
        }
    }

    // Example of sending data
    void exampleSendData(const std::string& data) {
        uint32_t nbytesWritten = 0;
        auto response = writeString(data, nbytesWritten);
        if (response == TTYResponse::OK) {
            std::cout << "Sent: " << data << " (" << nbytesWritten << " bytes)"
                      << std::endl;
        } else {
            std::cerr << "Failed to send data: " << getErrorMessage(response)
                      << std::endl;
        }
    }

    // Example of receiving data
    void exampleReceiveData(size_t size) {
        std::vector<uint8_t> buffer(size);
        uint32_t nbytesRead = 0;
        auto response = read(buffer.data(), size, 5, nbytesRead);
        if (response == TTYResponse::OK) {
            std::string receivedData(buffer.begin(),
                                     buffer.begin() + nbytesRead);
            std::cout << "Received: " << receivedData << " (" << nbytesRead
                      << " bytes)" << std::endl;
        } else {
            std::cerr << "Failed to receive data: " << getErrorMessage(response)
                      << std::endl;
        }
    }
};

int main() {
    // Create an instance of the TTY client
    MyTtyClient ttyClient("MyTTYDriver");

    // Example device name (update it to your actual device)
    const std::string device = "/dev/ttyUSB0";

    // Connect to the TTY device
    ttyClient.exampleConnect(device);

    // Send some data
    ttyClient.exampleSendData("Hello TTY!");

    // Receive some data
    ttyClient.exampleReceiveData(100);

    // Disconnect from the device if needed
    ttyClient.disconnect();

    return 0;
}
