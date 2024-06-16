#include "task/generator.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using namespace lithium;
using json = nlohmann::json;

class TaskGeneratorTest : public ::testing::Test {
protected:
    TaskGenerator generator;

    virtual void SetUp() {
        // Setup code if needed
    }

    virtual void TearDown() {
        // Cleanup code if needed
    }
};

TEST_F(TaskGeneratorTest, TestProcessJson) {
    json j = {{"name", "${name}"},
              {"email", "${email}"},
              {"greeting", "${concat(Hello, , ${name})}"},
              {"isEqual", "${equals(${name}, John Doe)}"},
              {"lengthOfName", "${length(${name})}"},
              {"upperName", "${uppercase(${name})}"}};

    generator.process_json(j);

    EXPECT_EQ(j["name"], "John Doe");
    EXPECT_EQ(j["email"], "john.doe@example.com");
    EXPECT_EQ(j["greeting"], "Hello John Doe");
    EXPECT_EQ(j["isEqual"], "true");
    EXPECT_EQ(j["lengthOfName"], "8");
    EXPECT_EQ(j["upperName"], "JOHN DOE");
}