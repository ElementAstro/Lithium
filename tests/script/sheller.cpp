#include "script/sheller.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>


using namespace lithium;

class ScriptManagerTest : public ::testing::Test {
protected:
    ScriptManager scriptManager;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ScriptManagerTest, RegisterScript) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    auto scripts = scriptManager.getAllScripts();
    ASSERT_TRUE(scripts.contains("test_script"));
    ASSERT_EQ(scripts["test_script"], script);
}

TEST_F(ScriptManagerTest, RegisterPowerShellScript) {
    Script script = "Write-Output 'Hello, World!'";
    scriptManager.registerPowerShellScript("test_ps_script", script);

    auto scripts = scriptManager.getAllScripts();
    ASSERT_TRUE(scripts.contains("test_ps_script"));
    ASSERT_EQ(scripts["test_ps_script"], script);
}

TEST_F(ScriptManagerTest, DeleteScript) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.deleteScript("test_script");

    auto scripts = scriptManager.getAllScripts();
    ASSERT_FALSE(scripts.contains("test_script"));
}

TEST_F(ScriptManagerTest, UpdateScript) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    Script updatedScript = "echo Updated Script";
    scriptManager.updateScript("test_script", updatedScript);

    auto scripts = scriptManager.getAllScripts();
    ASSERT_TRUE(scripts.contains("test_script"));
    ASSERT_EQ(scripts["test_script"], updatedScript);
}

TEST_F(ScriptManagerTest, RunScript) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    auto result = scriptManager.runScript("test_script", {});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->second, 0);  // Assuming 0 is the success status code
}

TEST_F(ScriptManagerTest, RunScriptWithArgs) {
    Script script = "echo $1";
    scriptManager.registerScript("test_script", script);

    std::unordered_map<std::string, std::string> args = {
        {"1", "Hello, World!"}};
    auto result = scriptManager.runScript("test_script", args);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->second, 0);  // Assuming 0 is the success status code
}

TEST_F(ScriptManagerTest, RunScriptWithTimeout) {
    Script script = "sleep 2 && echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    auto result = scriptManager.runScript("test_script", {}, true,
                                          1000);  // 1 second timeout
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->second, -1);  // Assuming -1 is the timeout status code
}

TEST_F(ScriptManagerTest, GetScriptOutput) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.runScript("test_script", {});
    auto output = scriptManager.getScriptOutput("test_script");
    ASSERT_TRUE(output.has_value());
    ASSERT_EQ(output.value(), "Hello, World!\n");
}

TEST_F(ScriptManagerTest, GetScriptStatus) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.runScript("test_script", {});
    auto status = scriptManager.getScriptStatus("test_script");
    ASSERT_TRUE(status.has_value());
    ASSERT_EQ(status.value(), 0);  // Assuming 0 is the success status code
}

TEST_F(ScriptManagerTest, RunScriptsSequentially) {
    Script script1 = "echo Script 1";
    Script script2 = "echo Script 2";
    scriptManager.registerScript("script1", script1);
    scriptManager.registerScript("script2", script2);

    std::vector<
        std::pair<std::string, std::unordered_map<std::string, std::string>>>
        scripts = {{"script1", {}}, {"script2", {}}};

    auto results = scriptManager.runScriptsSequentially(scripts);
    ASSERT_EQ(results.size(), 2);
    ASSERT_TRUE(results[0].has_value());
    ASSERT_TRUE(results[1].has_value());
    ASSERT_EQ(results[0]->second, 0);  // Assuming 0 is the success status code
    ASSERT_EQ(results[1]->second, 0);  // Assuming 0 is the success status code
}

TEST_F(ScriptManagerTest, RunScriptsConcurrently) {
    Script script1 = "echo Script 1";
    Script script2 = "echo Script 2";
    scriptManager.registerScript("script1", script1);
    scriptManager.registerScript("script2", script2);

    std::vector<
        std::pair<std::string, std::unordered_map<std::string, std::string>>>
        scripts = {{"script1", {}}, {"script2", {}}};

    auto results = scriptManager.runScriptsConcurrently(scripts);
    ASSERT_EQ(results.size(), 2);
    ASSERT_TRUE(results[0].has_value());
    ASSERT_TRUE(results[1].has_value());
    ASSERT_EQ(results[0]->second, 0);  // Assuming 0 is the success status code
    ASSERT_EQ(results[1]->second, 0);  // Assuming 0 is the success status code
}

TEST_F(ScriptManagerTest, EnableVersioning) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.enableVersioning();
    scriptManager.updateScript("test_script", "echo Updated Script");

    auto scripts = scriptManager.getAllScripts();
    ASSERT_TRUE(scripts.contains("test_script"));
    ASSERT_EQ(scripts["test_script"], "echo Updated Script");
}

TEST_F(ScriptManagerTest, RollbackScript) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.enableVersioning();
    scriptManager.updateScript("test_script", "echo Updated Script");

    bool rollbackSuccess = scriptManager.rollbackScript("test_script", 0);
    ASSERT_TRUE(rollbackSuccess);

    auto scripts = scriptManager.getAllScripts();
    ASSERT_TRUE(scripts.contains("test_script"));
    ASSERT_EQ(scripts["test_script"], "echo Hello, World!");
}

TEST_F(ScriptManagerTest, SetScriptCondition) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.setScriptCondition("test_script", []() { return false; });

    auto result = scriptManager.runScript("test_script", {});
    ASSERT_FALSE(result.has_value());
}

TEST_F(ScriptManagerTest, SetExecutionEnvironment) {
    Script script = "echo $MY_ENV_VAR";
    scriptManager.registerScript("test_script", script);

    scriptManager.setExecutionEnvironment("test_script", "MY_ENV_VAR=Hello");

    auto result = scriptManager.runScript("test_script", {});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->second, 0);  // Assuming 0 is the success status code
}

TEST_F(ScriptManagerTest, SetMaxScriptVersions) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    scriptManager.setMaxScriptVersions(1);
    scriptManager.updateScript("test_script", "echo Updated Script");

    auto scripts = scriptManager.getAllScripts();
    ASSERT_TRUE(scripts.contains("test_script"));
    ASSERT_EQ(scripts["test_script"], "echo Updated Script");

    bool rollbackSuccess = scriptManager.rollbackScript("test_script", 0);
    ASSERT_FALSE(rollbackSuccess);
}

TEST_F(ScriptManagerTest, GetScriptLogs) {
    Script script = "echo Hello, World!";
    scriptManager.registerScript("test_script", script);

    auto logs = scriptManager.getScriptLogs("test_script");
    ASSERT_FALSE(logs.empty());
    ASSERT_EQ(logs.back(), "Script registered/updated.");
}
