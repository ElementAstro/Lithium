#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "atom/task/loop_task.hpp"


// Define a mock function for the item function
json mock_item_fn(const json& j) {
    // Implement the mock logic for the item function
    return j;
}

// Define a mock function for the stop function
json mock_stop_fn(const json& j) {
    // Implement the mock logic for the stop function
    return j;
}

TEST(LoopTaskTest, ExecuteTest) {
    // Create a LoopTask object with mock functions and parameters
    LoopTask task(mock_item_fn, mock_stop_fn, json(), 5);

    // Call the execute function and check the result
    json result = task.execute();
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["loop_count"], 5);
    // Add more assertions if necessary
}

TEST(LoopTaskTest, ToJsonTest) {
    // Create a LoopTask object with mock functions and parameters
    LoopTask task(mock_item_fn, mock_stop_fn, json(), 5);

    // Call the toJson function and check the result
    json result = task.toJson();
    EXPECT_EQ(result["type"], "LoopTask");
    EXPECT_EQ(result["loop_count"], 5);
    // Add more assertions if necessary
}