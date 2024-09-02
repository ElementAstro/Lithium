/*
 * main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-10-01

Description: Example usage of the UdpClient class.

**************************************************/

#include <iostream>
#include <thread>

#include "atom/connection/udpclient.hpp"

// Function to handle incoming data
void onDataReceived(const std::vector<char>& data, const std::string& senderIp,
                    int senderPort) {
    std::string receivedData(data.begin(), data.end());
    std::cout << "Received data: '" << receivedData << "' from " << senderIp
              << ":" << senderPort << std::endl;
}

// Function to handle errors
void onError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}

// Function to run the UDP client
void runUdpClient(const std::string& host, int port) {
    atom::connection::UdpClient udpClient;

    // Set up callbacks
    udpClient.setOnDataReceivedCallback(onDataReceived);
    udpClient.setOnErrorCallback(onError);

    // Bind to a port for receiving
    if (!udpClient.bind(8080)) {  // Using port 8080 for receiving
        std::cerr << "Failed to bind UDP client to port 8080." << std::endl;
        return;
    }

    // Start receiving data
    udpClient.startReceiving(
        1024);  // Start receiving with a buffer size of 1024

    // Simulate sending a message to the server
    std::string message = "Hello, UDP Server!";
    if (udpClient.send(host, port,
                       std::vector<char>(message.begin(), message.end()))) {
        std::cout << "Sent message: " << message << std::endl;
    } else {
        std::cerr << "Failed to send message." << std::endl;
    }

    // Let it run for some time to receive responses
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Stop receiving data
    udpClient.stopReceiving();
}

int main() {
    const std::string host =
        "127.0.0.1";        // Replace with the server's IP address or hostname
    const int port = 8080;  // Replace with the server's port

    // Run the UDP client
    runUdpClient(host, port);

    return 0;
}
