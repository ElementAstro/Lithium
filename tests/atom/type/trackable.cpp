#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <vector>

#include "atom/type/trackable.hpp"

// Test initialization and basic assignment
TEST(TrackableTest, InitializationAndAssignment) {
    Trackable<int> trackable(10);
    EXPECT_EQ(trackable.get(), 10);

    trackable = 20;
    EXPECT_EQ(trackable.get(), 20);
}

// Test observer notification
TEST(TrackableTest, ObserverNotification) {
    Trackable<int> trackable(10);
    int oldVal = 0;
    int newVal = 0;

    trackable.subscribe([&](const int& oldValue, const int& newValue) {
        oldVal = oldValue;
        newVal = newValue;
    });

    trackable = 20;
    EXPECT_EQ(oldVal, 10);
    EXPECT_EQ(newVal, 20);
}

// Test arithmetic operations
TEST(TrackableTest, ArithmeticOperations) {
    Trackable<int> trackable(10);

    trackable += 5;
    EXPECT_EQ(trackable.get(), 15);

    trackable -= 3;
    EXPECT_EQ(trackable.get(), 12);

    trackable *= 2;
    EXPECT_EQ(trackable.get(), 24);

    trackable /= 4;
    EXPECT_EQ(trackable.get(), 6);
}

// Test deferred notifications
TEST(TrackableTest, DeferredNotifications) {
    Trackable<int> trackable(10);
    int oldVal = 0;
    int newVal = 0;

    trackable.subscribe([&](const int& oldValue, const int& newValue) {
        oldVal = oldValue;
        newVal = newValue;
    });

    {
        auto deferrer = trackable.deferScoped();
        trackable = 20;
        EXPECT_EQ(oldVal, 0);  // No notification yet
        EXPECT_EQ(newVal, 0);  // No notification yet
        trackable += 5;
        EXPECT_EQ(oldVal, 0);  // No notification yet
        EXPECT_EQ(newVal, 0);  // No notification yet
    }

    EXPECT_EQ(oldVal, 10);  // Notification after scope ends
    EXPECT_EQ(newVal, 25);  // Notification after scope ends
}

// Test vector append operation
TEST(TrackableTest, VectorAppendOperation) {
    Trackable<std::vector<int>> trackable({1, 2, 3});
    std::vector<int> oldVal;
    std::vector<int> newVal;

    trackable.subscribe([&](const std::vector<int>& oldValue,
                            const std::vector<int>& newValue) {
        oldVal = oldValue;
        newVal = newValue;
    });

    trackable += std::vector<int>{4, 5};
    EXPECT_EQ(oldVal, (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(newVal, (std::vector<int>{1, 2, 3, 4, 5}));
}

// Test unsubscribe all
TEST(TrackableTest, UnsubscribeAll) {
    Trackable<int> trackable(10);
    bool notified = false;

    trackable.subscribe([&](const int&, const int&) { notified = true; });

    trackable.unsubscribeAll();
    trackable = 20;
    EXPECT_FALSE(notified);
}

// Test type name retrieval
TEST(TrackableTest, TypeNameRetrieval) {
    Trackable<int> trackable(10);
    EXPECT_EQ(trackable.getTypeName(), "int");

    Trackable<std::string> trackableString("hello");
    EXPECT_EQ(trackableString.getTypeName(), "std::string");
}

// Test basic conversion operator
TEST(TrackableTest, ConversionOperator) {
    Trackable<int> trackable(10);
    int value = static_cast<int>(trackable);
    EXPECT_EQ(value, 10);
}