#include "atom/async/message_queue.hpp"

#include <chrono>
#include <iostream>
#include <thread>

// Message structure
struct MyMessage {
    std::string content;
};

// Subscriber function to handle incoming messages
void messageHandler(const MyMessage &msg) {
    std::cout << "Received message: " << msg.content << std::endl;
}

int main() {
    // Create a MessageQueue instance for MyMessage
    atom::async::MessageQueue<MyMessage> messageQueue;

    // Subscribe to the message queue
    messageQueue.subscribe(messageHandler, "MessageHandler");

    // Start the processing thread
    messageQueue.startProcessingThread();

    // Publish some messages to the queue
    for (int i = 0; i < 5; ++i) {
        MyMessage msg{"Hello World " + std::to_string(i)};
        messageQueue.publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(
            200));  // Simulate some delay between messages
    }

    // Allow some time for processing before stopping
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Stop the processing thread
    messageQueue.stopProcessingThread();

    return 0;
}
