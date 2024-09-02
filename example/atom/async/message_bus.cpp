#include <chrono>
#include <iostream>
#include <thread>

#include "atom/async/message_bus.hpp"

// Message structure
struct MyMessage {
    std::string content;
};

void subscriberFunction(const MyMessage &msg) {
    std::cout << "Received message: " << msg.content << std::endl;
}

void globalSubscriberFunction(const MyMessage &msg) {
    std::cout << "Global subscriber received: " << msg.content << std::endl;
}

int main() {
    // Create a MessageBus instance
    auto bus = atom::async::MessageBus::createShared();

    // Subscribe to a specific topic
    bus->subscribe<MyMessage>("my_topic", subscriberFunction);

    // Subscribe to a global topic
    bus->globalSubscribe<MyMessage>(globalSubscriberFunction);

    // Publish messages to the topic
    for (int i = 0; i < 5; ++i) {
        MyMessage msg{"Hello World " + std::to_string(i)};
        bus->publish("my_topic", msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(
            200));  // Simulate some delay between messages
    }

    // Publish a message after a delay
    std::this_thread::sleep_for(std::chrono::seconds(1));
    MyMessage globalMsg{"This is a global message!"};
    bus->publish("global_topic", globalMsg);

    // Delay to allow global subscribers to process messages
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Unsubscribe from the topic
    bus->unsubscribe<MyMessage>("my_topic", subscriberFunction);

    // Publish another message to see if the subscriber still receives it
    MyMessage msg{"This should NOT be received by the local subscriber!"};
    bus->publish("my_topic", msg);

    // Wait for a moment to observe potential output
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Stop all processing threads if any (not implemented here, just caution)
    // bus->stopAllProcessingThreads();

    return 0;
}
