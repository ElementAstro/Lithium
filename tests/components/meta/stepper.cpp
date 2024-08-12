#include <gtest/gtest.h>
#include <any>
#include <functional>
#include <string>
#include <vector>

#include "atom/function/stepper.hpp"

// Test suite for the FunctionSequence class
class FunctionSequenceTest : public ::testing::Test {
protected:
    atom::meta::FunctionSequence sequence;
};

// Test when no functions are registered
TEST_F(FunctionSequenceTest, NoFunctionsRegistered) {
    std::vector<std::vector<std::any>> args_batch = {{1}, {2}, {3}};

    EXPECT_THROW(sequence.run(args_batch), atom::error::Exception);
    EXPECT_THROW(sequence.run_all(args_batch), atom::error::Exception);
}

// Test with a single function that adds two integers
TEST_F(FunctionSequenceTest, SingleFunctionAddIntegers) {
    sequence.register_function(
        [](const std::vector<std::any>& args) -> std::any {
            int sum = 0;
            for (const auto& arg : args) {
                sum += std::any_cast<int>(arg);
            }
            return sum;
        });

    std::vector<std::vector<std::any>> args_batch = {{1, 2}, {3, 4}, {5, 6}};
    std::vector<std::any> results = sequence.run(args_batch);

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(std::any_cast<int>(results[0]), 3);
    EXPECT_EQ(std::any_cast<int>(results[1]), 7);
    EXPECT_EQ(std::any_cast<int>(results[2]), 11);
}

// Test with multiple functions
TEST_F(FunctionSequenceTest, MultipleFunctions) {
    sequence.register_function(
        [](const std::vector<std::any>& args) -> std::any {
            int sum = 0;
            for (const auto& arg : args) {
                sum += std::any_cast<int>(arg);
            }
            return sum;
        });

    sequence.register_function(
        [](const std::vector<std::any>& args) -> std::any {
            int product = 1;
            for (const auto& arg : args) {
                product *= std::any_cast<int>(arg);
            }
            return product;
        });

    std::vector<std::vector<std::any>> args_batch = {{2, 3}, {4, 5}, {6, 7}};
    std::vector<std::any> results = sequence.run(args_batch);

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(std::any_cast<int>(results[0]),
              6);  // Last function is product: 2 * 3
    EXPECT_EQ(std::any_cast<int>(results[1]), 20);  // 4 * 5
    EXPECT_EQ(std::any_cast<int>(results[2]), 42);  // 6 * 7

    std::vector<std::vector<std::any>> results_all =
        sequence.run_all(args_batch);

    ASSERT_EQ(results_all.size(), 3);
    EXPECT_EQ(std::any_cast<int>(results_all[0][0]),
              5);  // First function is sum: 2 + 3
    EXPECT_EQ(std::any_cast<int>(results_all[0][1]), 6);   // Product: 2 * 3
    EXPECT_EQ(std::any_cast<int>(results_all[1][0]), 9);   // 4 + 5
    EXPECT_EQ(std::any_cast<int>(results_all[1][1]), 20);  // 4 * 5
    EXPECT_EQ(std::any_cast<int>(results_all[2][0]), 13);  // 6 + 7
    EXPECT_EQ(std::any_cast<int>(results_all[2][1]), 42);  // 6 * 7
}

// Test function with different types of arguments
TEST_F(FunctionSequenceTest, MixedArgumentTypes) {
    sequence.register_function(
        [](const std::vector<std::any>& args) -> std::any {
            std::string result;
            for (const auto& arg : args) {
                if (arg.type() == typeid(int)) {
                    result += std::to_string(std::any_cast<int>(arg)) + " ";
                } else if (arg.type() == typeid(std::string)) {
                    result += std::any_cast<std::string>(arg) + " ";
                }
            }
            return result;
        });

    std::vector<std::vector<std::any>> args_batch = {{1, std::string("Hello")},
                                                     {2, std::string("World")},
                                                     {3, std::string("Test")}};
    std::vector<std::any> results = sequence.run(args_batch);

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(std::any_cast<std::string>(results[0]), "1 Hello ");
    EXPECT_EQ(std::any_cast<std::string>(results[1]), "2 World ");
    EXPECT_EQ(std::any_cast<std::string>(results[2]), "3 Test ");
}

// Test with an empty argument batch
TEST_F(FunctionSequenceTest, EmptyArgumentsBatch) {
    sequence.register_function([](const std::vector<std::any>&) -> std::any {
        return std::string("No Args");
    });

    std::vector<std::vector<std::any>> args_batch;
    std::vector<std::any> results = sequence.run(args_batch);

    ASSERT_TRUE(results.empty());

    std::vector<std::vector<std::any>> results_all =
        sequence.run_all(args_batch);
    ASSERT_TRUE(results_all.empty());
}

// Test with a function that throws an exception
TEST_F(FunctionSequenceTest, FunctionThrowsException) {
    sequence.register_function([](const std::vector<std::any>&) -> std::any {
        throw std::runtime_error("Test exception");
    });

    std::vector<std::vector<std::any>> args_batch = {{1, 2}, {3, 4}};

    EXPECT_THROW(sequence.run(args_batch), atom::error::Exception);
    EXPECT_THROW(sequence.run_all(args_batch), atom::error::Exception);
}

// Test when a function returns a different type
TEST_F(FunctionSequenceTest, DifferentReturnType) {
    sequence.register_function(
        [](const std::vector<std::any>& args) -> std::any {
            return std::any_cast<int>(args[0]) * 2;
        });

    sequence.register_function(
        [](const std::vector<std::any>& args) -> std::any {
            return std::to_string(std::any_cast<int>(args[0])) + "x2";
        });

    std::vector<std::vector<std::any>> args_batch = {{5}, {10}, {15}};
    std::vector<std::any> results = sequence.run(args_batch);

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(std::any_cast<std::string>(results[0]), "5x2");
    EXPECT_EQ(std::any_cast<std::string>(results[1]), "10x2");
    EXPECT_EQ(std::any_cast<std::string>(results[2]), "15x2");

    std::vector<std::vector<std::any>> results_all =
        sequence.run_all(args_batch);

    ASSERT_EQ(results_all.size(), 3);
    EXPECT_EQ(std::any_cast<int>(results_all[0][0]), 10);              // 5 * 2
    EXPECT_EQ(std::any_cast<std::string>(results_all[0][1]), "5x2");   // "5x2"
    EXPECT_EQ(std::any_cast<int>(results_all[1][0]), 20);              // 10 * 2
    EXPECT_EQ(std::any_cast<std::string>(results_all[1][1]), "10x2");  // "10x2"
    EXPECT_EQ(std::any_cast<int>(results_all[2][0]), 30);              // 15 * 2
    EXPECT_EQ(std::any_cast<std::string>(results_all[2][1]), "15x2");  // "15x2"
}
