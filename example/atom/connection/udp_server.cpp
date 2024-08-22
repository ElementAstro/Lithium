#include "atom/connection/udp_server.hpp"

#include <iostream>
#include <thread>

// Function to handle incoming messages
void onMessageReceived(const std::string& message, const std::string& senderIp,
                       int senderPort) {
    std::cout << "Received message: " << message << " from " << senderIp << ":"
              << senderPort << std::endl;
}

// Function to run the UDP server
void runUdpServer(int port) {
    atom::connection::UdpSocketHub udpServer;

    // Add message handler
    udpServer.addMessageHandler(onMessageReceived);

    // Start the UDP server
    udpServer.start(port);
    std::cout << "UDP server started on port " << port << std::endl;

    // Keep the server running for a while to receive messages
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Stop the UDP server
    udpServer.stop();
    std::cout << "UDP server stopped." << std::endl;
}

int main() {
    const int port = 8080;  // Port to listen for incoming messages

    // Run the UDP server in a thread
    std::thread serverThread(runUdpServer, port);

    // Wait for the server thread to finish
    serverThread.join();

    return 0;
}
