#include <gtest/gtest.h>
#include "task/manager.hpp"

using namespace lithium;

class TaskInterpretorTest : public ::testing::Test {
protected:
    void SetUp() override { interpretor = std::make_unique<TaskInterpretor>(); }

    void TearDown() override { interpretor.reset(); }

    std::unique_ptr<TaskInterpretor> interpretor;
};

TEST_F(TaskInterpretorTest, LoadScript) {
    std::string scriptName = "test_script";
    json script = {{"key", "value"}};

    interpretor->loadScript(scriptName, script);

    // Verify that the script is loaded correctly
    EXPECT_TRUE(interpretor->hasScript(scriptName));
    EXPECT_EQ(interpretor->getScript(scriptName), script);
}

TEST_F(TaskInterpretorTest, RegisterFunction) {
    std::string functionName = "test_function";
    std::function<json(const json&)> func = [](const json& input) {
        return input;
    };

    interpretor->registerFunction(functionName, func);

    // Verify that the function is registered correctly

    EXPECT_TRUE(interpretor->hasFunction(functionName));
}
