#include <iostream>
#include <thread>

#include "atom/connection/sockethub.hpp"

// Function to handle incoming messages
void messageHandler(std::string message) {
    std::cout << "Received message: " << message << std::endl;
}

// Function to run the socket server
void runSocketServer(int port) {
    atom::connection::SocketHub socketHub;

    // Add a custom message handler
    socketHub.addHandler(messageHandler);

    // Start the socket server
    socketHub.start(port);
    std::cout << "Socket server running on port " << port << std::endl;

    // Run for a specific duration and then stop the server
    std::this_thread::sleep_for(std::chrono::seconds(30));
    socketHub.stop();
    std::cout << "Socket server stopped." << std::endl;
}

int main() {
    const int port = 8080;  // Define the port to listen on

    // Start the socket server in a separate thread
    std::thread serverThread(runSocketServer, port);

    // Wait for the server thread to finish
    serverThread.join();

    return 0;
}
