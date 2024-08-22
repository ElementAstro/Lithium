#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "atom/connection/fifoserver.hpp"

// Function to run the FIFO server
void runFifoServer(const std::string& fifoPath) {
    atom::connection::FIFOServer server(fifoPath);

    // Start the server
    server.start();
    std::cout << "FIFO Server started." << std::endl;

    // Simulate sending messages
    for (int i = 0; i < 5; ++i) {
        std::string message = "Message " + std::to_string(i);
        server.sendMessage(message);
        std::cout << "Sent: " << message << std::endl;

        // Sleep for a while to simulate some processing time
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Stop the server
    server.stop();
    std::cout << "FIFO Server stopped." << std::endl;
}

int main() {
    const std::string fifoPath = "/tmp/my_fifo";  // Path for the FIFO

    // Create a thread to run the FIFO server
    std::thread serverThread(runFifoServer, fifoPath);

    // Wait for the server thread to finish
    serverThread.join();

    return 0;
}
