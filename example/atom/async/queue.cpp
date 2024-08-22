#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "atom/async/queue.hpp"

// Function to simulate a producer that adds messages to the queue
void producer(atom::async::ThreadSafeQueue<std::string> &queue) {
    for (int i = 0; i < 10; ++i) {
        std::string message = "Message " + std::to_string(i);
        queue.put(message);
        std::cout << "Produced: " << message << std::endl;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(200));  // Simulate work
    }
}

// Function to simulate a consumer that takes messages from the queue
void consumer(atom::async::ThreadSafeQueue<std::string> &queue) {
    for (int i = 0; i < 10; ++i) {
        auto message = queue.take();
        if (message) {
            std::cout << "Consumed: " << *message << std::endl;
        } else {
            std::cout << "No message taken!" << std::endl;
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(300));  // Simulate processing delay
    }
}

int main() {
    atom::async::ThreadSafeQueue<std::string> messageQueue;

    // Create producer and consumer threads
    std::thread producerThread(producer, std::ref(messageQueue));
    std::thread consumerThread(consumer, std::ref(messageQueue));

    // Wait for both threads to finish
    producerThread.join();
    consumerThread.join();

    std::cout << "Processing complete." << std::endl;

    return 0;
}
