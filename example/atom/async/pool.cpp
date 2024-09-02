#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "atom/async/pool.hpp"

// A sample task function that simulates work
void sampleTask(int id) {
    std::cout << "Task " << id << " is starting on thread "
              << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));  // Simulate work
    std::cout << "Task " << id << " completed on thread "
              << std::this_thread::get_id() << std::endl;
}

int main() {
    const unsigned int numThreads = 4;  // Number of threads in the pool
    atom::async::ThreadPool<> threadPool(
        numThreads);  // Create ThreadPool instance

    std::vector<std::future<void>>
        futures;  // To hold futures for result checking

    // Enqueue multiple tasks into the thread pool
    for (int i = 0; i < 10; ++i) {
        futures.push_back(threadPool.enqueue(sampleTask, i));
    }

    // Wait for all tasks to complete
    for (auto &future : futures) {
        future.wait();
    }

    std::cout << "All tasks completed." << std::endl;

    return 0;
}
