#include <iostream>
#include <string>

#include "atom/async/eventstack.hpp"

// Define a simple event type (in this case, a string)
using EventType = std::string;

void exampleUsage() {
    // Create an EventStack for managing string events
    atom::async::EventStack<EventType> eventStack;

    // Push some events onto the stack
    eventStack.pushEvent("Event 1: Start processing data");
    eventStack.pushEvent("Event 2: Load configuration");
    eventStack.pushEvent("Event 3: Connect to database");
    eventStack.pushEvent("Event 4: Process user input");

    // Print size of the stack
    std::cout << "Current stack size: " << eventStack.size() << std::endl;

    // Peek at the top event
    auto topEvent = eventStack.peekTopEvent();
    if (topEvent) {
        std::cout << "Top event: " << *topEvent << std::endl;
    } else {
        std::cout << "Stack is empty!" << std::endl;
    }

    // Pop an event from the stack
    auto poppedEvent = eventStack.popEvent();
    if (poppedEvent) {
        std::cout << "Popped event: " << *poppedEvent << std::endl;
    } else {
        std::cout << "Stack is empty!" << std::endl;
    }

    // Filter events that contain the word "data"
    eventStack.filterEvents([](const EventType& event) {
        return event.find("data") != std::string::npos;
    });

    std::cout << "After filtering, stack size: " << eventStack.size()
              << std::endl;

#if ENABLE_DEBUG
    // Print remaining events
    eventStack.printEvents();
#endif

    // Serialize the stack to a string
    std::string serializedData = eventStack.serializeStack();
    std::cout << "Serialized stack: " << serializedData << std::endl;

    // Clear the stack, and then deserialize the serialized data back into the
    // stack
    eventStack.clearEvents();
    std::cout << "Stack cleared." << std::endl;

    eventStack.deserializeStack(serializedData);
    std::cout << "Deserialized stack size: " << eventStack.size() << std::endl;

    // Remove duplicates (if any)
    eventStack.removeDuplicates();

    // Sort events in the stack (lexicographical order)
    eventStack.sortEvents(
        [](const EventType& a, const EventType& b) { return a < b; });
    std::cout << "Sorted stack size: " << eventStack.size() << std::endl;

    // Check if any event contains the word "input"
    bool hasInputEvent = eventStack.anyEvent([](const EventType& event) {
        return event.find("input") != std::string::npos;
    });

    std::cout << (hasInputEvent ? "There is an event containing 'input'.\n"
                                : "No events contain 'input'.\n");
}

int main() {
    // Run the event stack example
    exampleUsage();
    return 0;
}
