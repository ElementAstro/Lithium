#include "checker.hpp"

#include <fstream>
#include <regex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#include <windows.h>
#endif

#include "atom/error/exception.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/macro.hpp"

using json = nlohmann::json;

namespace lithium {

struct DangerItem {
    std::string category;
    std::string command;
    std::string reason;
    int line;
    std::optional<std::string> context;
} ATOM_ALIGNAS(128);

class ScriptAnalyzerImpl {
public:
    explicit ScriptAnalyzerImpl(const std::string& config_file) {
        config_ = loadConfig(config_file);
    }

    void analyze(const std::string& script, bool output_json = false) {
        std::vector<DangerItem> dangers;
        detectScriptTypeAndAnalyze(script, dangers);
        suggestSafeReplacements(script, dangers);
        int complexity = calculateComplexity(script);
        generateReport(dangers, complexity, output_json);
    }

private:
    json config_;
    mutable std::shared_mutex config_mutex_;

    auto loadConfig(const std::string& config_file) -> json {
        if (!atom::io::isFileExists(config_file)) {
            THROW_FILE_NOT_FOUND("Config file not found: " + config_file);
            return json::object();
        }
        std::ifstream file(config_file);
        if (!file.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Unable to open config file: " +
                                    config_file);
        }
        json config;
        file >> config;
        return config;
    }

    static auto isSkippableLine(const std::string& line) -> bool {
        return line.empty() ||
               std::regex_match(line, std::regex(R"(^\s*#.*)")) ||
               std::regex_match(
                   line, std::regex(R"(^\s*//.*)"));  // 支持PowerShell注释
    }

    void detectScriptTypeAndAnalyze(const std::string& script,
                                    std::vector<DangerItem>& dangers) {
        std::shared_lock lock(config_mutex_);

#ifdef _WIN32
        bool isPowerShell = detectPowerShell(script);
        if (isPowerShell) {
            checkPattern(script, config_["powershell_danger_patterns"],
                         "PowerShell Security Issue", dangers);
        } else {
            checkPattern(script, config_["windows_cmd_danger_patterns"],
                         "CMD Security Issue", dangers);
        }
#else
        checkPattern(script, config_["bash_danger_patterns"],
                     "Shell Script Security Issue", dangers);
#endif
    }

    static bool detectPowerShell(const std::string& script) {
        return script.find("param(") !=
                   std::string::npos ||  // PowerShell 参数化的典型特征
               script.find("$PSVersionTable") !=
                   std::string::npos;  // 检测PowerShell的版本信息
    }

    void suggestSafeReplacements(const std::string& script,
                                 std::vector<DangerItem>& dangers) {
        std::unordered_map<std::string, std::string> replacements = {
#ifdef _WIN32
            {"Remove-Item -Recurse -Force",
             "Remove-Item -Recurse"},  // PowerShell危险命令替换
            {"Stop-Process -Force", "Stop-Process"},
#else
            {"rm -rf /", "find . -type f -delete"},
            {"kill -9", "kill -TERM"},
#endif
        };
        checkReplacements(script, replacements, dangers);
    }

    static auto calculateComplexity(const std::string& script) -> int {
        std::regex complexityPatterns(R"(if\b|while\b|for\b|case\b|&&|\|\|)");
        std::istringstream scriptStream(script);
        std::string line;
        int complexity = 0;

        while (std::getline(scriptStream, line)) {
            if (std::regex_search(line, complexityPatterns)) {
                complexity++;
            }
        }

        return complexity;
    }

    static void generateReport(const std::vector<DangerItem>& dangers,
                               int complexity, bool output_json) {
        if (output_json) {
            json report = json::object();
            report["complexity"] = complexity;
            report["issues"] = json::array();

            for (const auto& item : dangers) {
                report["issues"].push_back(
                    {{"category", item.category},
                     {"line", item.line},
                     {"command", item.command},
                     {"reason", item.reason},
                     {"context", item.context.value_or("")}});
            }
            LOG_F(INFO, "Generating JSON report: {}", report.dump(4));
        } else {
            LOG_F(INFO, "Shell Script Analysis Report");
            LOG_F(INFO, "============================");
            LOG_F(INFO, "Code Complexity: {}", complexity);

            if (dangers.empty()) {
                LOG_F(INFO, "No potential dangers found.");
            } else {
                for (const auto& item : dangers) {
                    LOG_F(INFO,
                          "Category: {}\n Line: {}\n Command: {}\n Reason: "
                          "{}\n Context: {}",
                          item.category, item.line, item.command, item.reason,
                          item.context.value_or(""));
                }
            }
        }
    }

    static void checkPattern(const std::string& script, const json& patterns,
                             const std::string& category,
                             std::vector<DangerItem>& dangers) {
        std::unordered_set<std::string> detectedIssues;
        std::istringstream scriptStream(script);
        std::string line;
        int lineNum = 0;

        while (std::getline(scriptStream, line)) {
            lineNum++;
            if (isSkippableLine(line)) {
                continue;
            }

            for (const auto& item : patterns) {
                std::regex pattern(item["pattern"]);
                std::string reason = item["reason"];

                if (std::regex_search(line, pattern)) {
                    std::string key = std::to_string(lineNum) + ":" + reason;
                    if (!detectedIssues.contains(key)) {
                        dangers.push_back(
                            {category, line, reason, lineNum, {}});
                        detectedIssues.insert(key);
                    }
                }
            }
        }
    }

    static void checkReplacements(
        const std::string& script,
        const std::unordered_map<std::string, std::string>& replacements,
        std::vector<DangerItem>& dangers) {
        std::istringstream scriptStream(script);
        std::string line;
        int lineNum = 0;
        std::unordered_set<std::string> detectedIssues;

        while (std::getline(scriptStream, line)) {
            lineNum++;
            if (isSkippableLine(line)) {
                continue;
            }

            for (const auto& [unsafe_command, safe_command] : replacements) {
                if (line.find(unsafe_command) != std::string::npos) {
                    std::string key =
                        std::to_string(lineNum) + ":" + unsafe_command;
                    if (!detectedIssues.contains(key)) {
                        dangers.push_back(
                            {"Suggestion",
                             line,
                             "Consider replacing with: " + safe_command,
                             lineNum,
                             {}});
                        detectedIssues.insert(key);
                    }
                }
            }
        }
    }
};

ScriptAnalyzer::ScriptAnalyzer(const std::string& config_file)
    : impl_(std::make_unique<ScriptAnalyzerImpl>(config_file)) {}

void ScriptAnalyzer::analyze(const std::string& script, bool output_json) {
    impl_->analyze(script, output_json);
}

}  // namespace lithium
