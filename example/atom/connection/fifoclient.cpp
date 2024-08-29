#include <chrono>
#include <iostream>
#include <thread>

#include "atom/connection/fifoclient.hpp"

#if __linux
#include <sys/stat.h>
#endif

// Function to simulate the FIFO server
void fifoServer(const std::string& fifoPath) {
    // Open the FIFO for writing. If it does not exist, create it.
    mkfifo(fifoPath.c_str(),
           0666);  // Create the named pipe if it doesn't exist

    // Simulate a server writing to the FIFO
    atom::connection::FifoClient fifoClient(fifoPath);
    if (!fifoClient.isOpen()) {
        std::cerr << "Failed to open FIFO for writing." << std::endl;
        return;
    }

    std::string message = "Hello from FIFO Server!";
    fifoClient.write(message,
                     std::chrono::milliseconds(1000));  // Write with timeout
    std::cout << "Server wrote: " << message << std::endl;

    fifoClient.close();  // Close FIFO after writing
}

// Function to simulate the FIFO client
void fifoClient(const std::string& fifoPath) {
    // Create a FifoClient to read from the FIFO
    atom::connection::FifoClient fifoClient(fifoPath);
    if (!fifoClient.isOpen()) {
        std::cerr << "Failed to open FIFO for reading." << std::endl;
        return;
    }

    // Read from FIFO with a timeout
    auto data = fifoClient.read(std::chrono::milliseconds(5000));
    if (data) {
        std::cout << "Client read: " << *data << std::endl;
    } else {
        std::cerr << "Client failed to read data from FIFO." << std::endl;
    }

    fifoClient.close();  // Close FIFO after reading
}

int main() {
    const std::string fifoPath = "/tmp/myfifo";  // FIFO path

    // Create threads to simulate server and client
    std::thread serverThread(fifoServer, fifoPath);
    std::this_thread::sleep_for(std::chrono::milliseconds(
        100));  // Small delay to ensure server starts first
    std::thread clientThread(fifoClient, fifoPath);

    // Wait for both threads to finish
    serverThread.join();
    clientThread.join();

    return 0;
}
