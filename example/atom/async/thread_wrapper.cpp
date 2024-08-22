#include <chrono>
#include <iostream>
#include <thread>

#include "atom/async/thread_wrapper.hpp"

// A sample function to be executed in a thread
void threadFunction(int id, std::chrono::milliseconds duration) {
    std::cout << "Thread " << id << " started. Sleeping for "
              << duration.count() << "ms.\n";
    std::this_thread::sleep_for(duration);
    std::cout << "Thread " << id << " finished processing!\n";
}

// A sample function that supports stopping
void stoppableThreadFunction(std::stop_token stopToken) {
    for (int i = 0; i < 5; ++i) {
        if (stopToken.stop_requested()) {
            std::cout << "Thread is stopping early!\n";
            return;
        }
        std::cout << "Working... " << i + 1 << "\n";
        std::this_thread::sleep_for(
            std::chrono::milliseconds(500));  // Simulate work
    }
}

int main() {
    // Create a Thread for normal execution
    atom::async::Thread normalThread;
    normalThread.start(threadFunction, 1, std::chrono::milliseconds(2000));
    normalThread.join();  // Wait for it to finish

    // Create a Thread that can be stopped
    atom::async::Thread stoppableThread;
    stoppableThread.start(stoppableThreadFunction);  // Start a stoppable thread

    // Give it some time to work
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Requesting the stoppable thread to stop...\n";
    stoppableThread.requestStop();  // Request it to stop

    stoppableThread.join();  // Wait for it to finish

    return 0;
}
