#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include "atom/async/async.hpp"

// Sample function to be run asynchronously
int sampleTask(int duration) {
    std::this_thread::sleep_for(std::chrono::seconds(duration));
    return duration;  // Return the duration as result
}

int main() {
    // Create an AsyncWorker object for managing asynchronous tasks
    atom::async::AsyncWorker<int> worker;

    // Start an asynchronous task
    worker.startAsync(sampleTask, 3);  // This will sleep for 3 seconds

    // Set a callback to handle the result when the task is done
    worker.setCallback([](int result) {
        std::cout << "Task completed with result: " << result << std::endl;
    });

    // Set a timeout of 5 seconds
    worker.setTimeout(std::chrono::seconds(5));

    // Wait for completion
    std::cout << "Waiting for task completion...\n";
    worker.waitForCompletion();

    // Get the result (this will work since we know the task completed)
    try {
        int result = worker.getResult();
        std::cout << "Result retrieved successfully: " << result << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error retrieving result: " << e.what() << std::endl;
    }

    // Using AsyncWorkerManager to manage multiple workers
    atom::async::AsyncWorkerManager<int> manager;

    // Create multiple async workers
    manager.createWorker(sampleTask, 1);  // 1 second task
    manager.createWorker(sampleTask, 2);  // 2 seconds task
    manager.createWorker(sampleTask, 3);  // 3 seconds task

    // Wait for all created tasks to complete
    std::cout << "Waiting for all tasks to complete...\n";
    manager.waitForAll();

    // Check if all tasks are done
    if (manager.allDone()) {
        std::cout << "All tasks have completed successfully.\n";
    } else {
        std::cout << "Some tasks are still running.\n";
    }

    // Retry logic using asyncRetry for a task that may fail
    auto retryExample = [](int x) {
        static int attempt = 0;
        attempt++;
        if (attempt < 3) {
            std::cerr << "Attempt " << attempt << " failed, retrying...\n";
            throw std::runtime_error("Simulated failure");
        }
        return x * 2;  // Successful result
    };

    // Execute with retry
    std::future<int> futureResult = atom::async::asyncRetry(
        retryExample, 3, std::chrono::milliseconds(500), 5);
    try {
        int finalResult = futureResult.get();
        std::cout << "Final result after retrying: " << finalResult
                  << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error after retries: " << e.what() << std::endl;
    }

    return 0;
}
