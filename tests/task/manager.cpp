#include <gtest/gtest.h>
#include "task/manager.hpp"

#include "atom/type/json.hpp"

using namespace lithium;
using json = nlohmann::json;

class TaskInterpreterTest : public ::testing::Test {
protected:
    void SetUp() override { interpretor = std::make_unique<TaskInterpreter>(); }

    void TearDown() override { interpretor.reset(); }

    std::unique_ptr<TaskInterpreter> interpretor;
};

TEST_F(TaskInterpreterTest, LoadScript) {
    std::string scriptName = "test_script";
    json script = {{"key", "value"}};

    interpretor->loadScript(scriptName, script);

    // Verify that the script is loaded correctly
    EXPECT_TRUE(interpretor->hasScript(scriptName));
    EXPECT_EQ(interpretor->getScript(scriptName), script);
}

TEST_F(TaskInterpreterTest, RegisterFunction) {
    std::string functionName = "test_function";
    std::function<json(const json&)> func = [](const json& input) {
        return input;
    };

    interpretor->registerFunction(functionName, func);
}
