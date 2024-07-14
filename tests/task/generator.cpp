#include "task/generator.hpp"
#include <gtest/gtest.h>
#include "atom/type/json.hpp"

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
              {"greeting", "${concat(Hello , , ${name})}"},
              {"isEqual", "${equals(${name}, John Doe)}"},
              {"lengthOfName", "${length(${name})}"},
              {"upperName", "${uppercase(${name})}"}};

    generator.processJson(j);

    EXPECT_EQ(j["name"].get<std::string>(), "John Doe");
    EXPECT_EQ(j["email"].get<std::string>(), "john.doe@example.com");
    EXPECT_EQ(j["greeting"].get<std::string>(), "HelloJohn Doe");
    EXPECT_EQ(j["isEqual"].get<std::string>(), "true");
    EXPECT_EQ(j["lengthOfName"].get<std::string>(), "8");
    EXPECT_EQ(j["upperName"].get<std::string>(), "JOHN DOE");
}