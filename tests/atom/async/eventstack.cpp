#include "atom/async/eventstack.hpp"
#include <gtest/gtest.h>

TEST(EventStackTest, PushEvent) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(stack.size(), 3);
}

TEST(EventStackTest, PopEvent) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(stack.popEvent().value(), 3);
    ASSERT_EQ(stack.popEvent().value(), 2);
    ASSERT_EQ(stack.popEvent().value(), 1);
    ASSERT_TRUE(stack.popEvent().has_value() == false);
}

TEST(EventStackTest, IsEmpty) {
    atom::async::EventStack<int> stack;

    ASSERT_TRUE(stack.isEmpty());

    stack.pushEvent(1);
    ASSERT_FALSE(stack.isEmpty());
}

TEST(EventStackTest, Size) {
    atom::async::EventStack<int> stack;

    ASSERT_EQ(stack.size(), 0);

    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(stack.size(), 3);
}

TEST(EventStackTest, ClearEvents) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(stack.size(), 3);

    stack.clearEvents();

    ASSERT_EQ(stack.size(), 0);
}

TEST(EventStackTest, PeekTopEvent) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(stack.peekTopEvent().value(), 3);
    ASSERT_EQ(stack.size(), 3);
}

TEST(EventStackTest, CopyStack) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    atom::async::EventStack<int> copiedStack = stack.copyStack();

    ASSERT_EQ(copiedStack.size(), 3);
    ASSERT_EQ(copiedStack.peekTopEvent().value(), 3);
}

TEST(EventStackTest, FilterEvents) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    stack.filterEvents([](const int& event) { return event % 2 == 0; });

    ASSERT_EQ(stack.size(), 1);
    ASSERT_EQ(stack.peekTopEvent().value(), 2);
}

TEST(EventStackTest, SerializeStack) {
    atom::async::EventStack<std::string> stack;
    stack.pushEvent("event1");
    stack.pushEvent("event2");
    stack.pushEvent("event3");

    std::string serializedStack = stack.serializeStack();

    ASSERT_EQ(serializedStack, "event1;event2;event3;");
}

TEST(EventStackTest, DeserializeStack) {
    atom::async::EventStack<std::string> stack;
    std::string serializedData = "event1;event2;event3;";

    stack.deserializeStack(serializedData);

    ASSERT_EQ(stack.size(), 3);
    ASSERT_EQ(stack.peekTopEvent().value(), "event3");
}

TEST(EventStackTest, RemoveDuplicates) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(2);
    stack.pushEvent(3);
    stack.pushEvent(3);

    ASSERT_EQ(stack.size(), 5);

    stack.removeDuplicates();

    ASSERT_EQ(stack.size(), 3);
}

TEST(EventStackTest, SortEvents) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(3);
    stack.pushEvent(1);
    stack.pushEvent(2);

    stack.sortEvents([](const int& a, const int& b) { return a < b; });

    ASSERT_EQ(stack.peekTopEvent().value(), 1);
}

TEST(EventStackTest, ReverseEvents) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    stack.reverseEvents();

    ASSERT_EQ(stack.peekTopEvent().value(), 3);
}

TEST(EventStackTest, CountEvents) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(stack.countEvents([](const int& event) { return event == 2; }),
              2);
}

TEST(EventStackTest, FindEvent) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_EQ(
        stack.findEvent([](const int& event) { return event == 2; }).value(),
        2);
}

TEST(EventStackTest, AnyEvent) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_TRUE(stack.anyEvent([](const int& event) { return event > 2; }));
}

TEST(EventStackTest, AllEvents) {
    atom::async::EventStack<int> stack;
    stack.pushEvent(1);
    stack.pushEvent(2);
    stack.pushEvent(3);

    ASSERT_TRUE(stack.allEvents([](const int& event) { return event >= 1; }));
}
