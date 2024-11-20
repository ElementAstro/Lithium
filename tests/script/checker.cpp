#include "script/checker.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <string>

using namespace lithium;

class ScriptAnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary config file for testing
        std::ofstream configFile("test_config.json");
        configFile << R"({
            "powershell_danger_patterns": [
                {"pattern": "Remove-Item -Recurse -Force", "reason": "Dangerous command"}
            ],
            "windows_cmd_danger_patterns": [
                {"pattern": "del /F /Q", "reason": "Dangerous command"}
            ],
            "python_danger_patterns": [
                {"pattern": "import os", "reason": "Potentially dangerous import"}
            ],
            "ruby_danger_patterns": [
                {"pattern": "require 'open-uri'", "reason": "Potentially dangerous import"}
            ],
            "bash_danger_patterns": [
                {"pattern": "rm -rf /", "reason": "Dangerous command"}
            ]
        })";
        configFile.close();
    }

    void TearDown() override {
        // Remove the temporary config file
        std::remove("test_config.json");
    }
};

TEST_F(ScriptAnalyzerTest, LoadConfigValidFile) {
    ASSERT_NO_THROW({ ScriptAnalyzer analyzer("test_config.json"); });
}

TEST_F(ScriptAnalyzerTest, LoadConfigInvalidFile) {
    ASSERT_THROW(
        { ScriptAnalyzer analyzer("invalid_config.json"); },
        std::runtime_error);
}

TEST_F(ScriptAnalyzerTest, AnalyzePowerShellScript) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "Remove-Item -Recurse -Force";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzePythonScript) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "import os";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzeRubyScript) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "require 'open-uri'";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzeBashScript) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "rm -rf /";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzeComplexScript) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = R"(
        import os
        def dangerous_function():
            os.system('rm -rf /')
    )";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzeScriptWithExternalCommands) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "curl http://example.com";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzeScriptWithEnvironmentVariables) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "echo $HOME";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}

TEST_F(ScriptAnalyzerTest, AnalyzeScriptWithFileOperations) {
    ScriptAnalyzer analyzer("test_config.json");
    std::string script = "open('file.txt', 'r')";

    ASSERT_NO_THROW({ analyzer.analyze(script, true, ReportFormat::JSON); });
}
