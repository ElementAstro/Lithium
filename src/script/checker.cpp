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
#include "atom/macro.hpp"
#include "atom/type/json.hpp"

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
        try {
            config_ = loadConfig(config_file);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to initialize ScriptAnalyzerImpl: {}",
                  e.what());
            throw;
        }
    }

    void analyze(const std::string& script, bool output_json,
                 ReportFormat format) {
        try {
            std::vector<DangerItem> dangers;
            detectScriptTypeAndAnalyze(script, dangers);
            suggestSafeReplacements(script, dangers);
            detectExternalCommands(script, dangers);
            detectEnvironmentVariables(script, dangers);
            detectFileOperations(script, dangers);
            int complexity = calculateComplexity(script);
            generateReport(dangers, complexity, output_json, format);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Analysis failed: {}", e.what());
            throw;
        }
    }

private:
    json config_;
    mutable std::shared_mutex config_mutex_;

    static auto loadConfig(const std::string& config_file) -> json {
        if (!atom::io::isFileExists(config_file)) {
            THROW_FILE_NOT_FOUND("Config file not found: " + config_file);
        }
        std::ifstream file(config_file);
        if (!file.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Unable to open config file: " +
                                    config_file);
        }
        json config;
        try {
            file >> config;
        } catch (const json::parse_error& e) {
            THROW_INVALID_FORMAT("Invalid JSON format in config file: " +
                                 config_file);
        }
        return config;
    }

    static auto loadConfigFromDatabase(const std::string& db_file) -> json {
        if (!atom::io::isFileExists(db_file)) {
            THROW_FILE_NOT_FOUND("Database file not found: " + db_file);
        }
        std::ifstream file(db_file);
        if (!file.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Unable to open database file: " + db_file);
        }
        json db;
        try {
            file >> db;
        } catch (const json::parse_error& e) {
            THROW_INVALID_FORMAT("Invalid JSON format in database file: " +
                                 db_file);
        }
        return db;
    }

    static auto isSkippableLine(const std::string& line) -> bool {
        return line.empty() ||
               std::regex_match(line, std::regex(R"(^\s*#.*)")) ||
               std::regex_match(line, std::regex(R"(^\s*//.*)"));
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
        if (detectPython(script)) {
            checkPattern(script, config_["python_danger_patterns"],
                         "Python Script Security Issue", dangers);
        } else if (detectRuby(script)) {
            checkPattern(script, config_["ruby_danger_patterns"],
                         "Ruby Script Security Issue", dangers);
        } else {
            checkPattern(script, config_["bash_danger_patterns"],
                         "Shell Script Security Issue", dangers);
        }
#endif
    }

    static bool detectPowerShell(const std::string& script) {
        return script.contains("param(") || script.contains("$PSVersionTable");
    }

    static bool detectPython(const std::string& script) {
        return script.contains("import ") || script.contains("def ");
    }

    static bool detectRuby(const std::string& script) {
        return script.contains("require ") || script.contains("def ");
    }

    void suggestSafeReplacements(const std::string& script,
                                 std::vector<DangerItem>& dangers) {
        std::unordered_map<std::string, std::string> replacements = {
#ifdef _WIN32
            {"Remove-Item -Recurse -Force", "Remove-Item -Recurse"},
            {"Stop-Process -Force", "Stop-Process"},
#else
            {"rm -rf /", "find . -type f -delete"},
            {"kill -9", "kill -TERM"},
#endif
        };
        checkReplacements(script, replacements, dangers);
    }

    void detectExternalCommands(const std::string& script,
                                std::vector<DangerItem>& dangers) {
        std::unordered_set<std::string> externalCommands = {
#ifdef _WIN32
            "Invoke-WebRequest",
            "Invoke-RestMethod",
#else
            "curl",
            "wget",
#endif
        };
        checkExternalCommands(script, externalCommands, dangers);
    }

    void detectEnvironmentVariables(const std::string& script,
                                    std::vector<DangerItem>& dangers) {
        std::regex envVarPattern(R"(\$\{?[A-Za-z_][A-Za-z0-9_]*\}?)");
        checkPattern(script, envVarPattern, "Environment Variable Usage",
                     dangers);
    }

    void detectFileOperations(const std::string& script,
                              std::vector<DangerItem>& dangers) {
        std::regex fileOpPattern(
            R"(\b(open|read|write|close|unlink|rename)\b)");
        checkPattern(script, fileOpPattern, "File Operation", dangers);
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
                               int complexity, bool output_json,
                               ReportFormat format) {
        switch (format) {
            case ReportFormat::JSON:
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
                }
                break;
            case ReportFormat::XML:
                LOG_F(INFO, "<Report>");
                LOG_F(INFO, "  <Complexity>{}</Complexity>", complexity);
                LOG_F(INFO, "  <Issues>");
                for (const auto& item : dangers) {
                    LOG_F(INFO, "    <Issue>");
                    LOG_F(INFO, "      <Category>{}</Category>", item.category);
                    LOG_F(INFO, "      <Line>{}</Line>", item.line);
                    LOG_F(INFO, "      <Command>{}</Command>", item.command);
                    LOG_F(INFO, "      <Reason>{}</Reason>", item.reason);
                    LOG_F(INFO, "      <Context>{}</Context>",
                          item.context.value_or(""));
                    LOG_F(INFO, "    </Issue>");
                }
                LOG_F(INFO, "  </Issues>");
                LOG_F(INFO, "</Report>");
                break;
            case ReportFormat::TEXT:
            default:
                LOG_F(INFO, "Shell Script Analysis Report");
                LOG_F(INFO, "============================");
                LOG_F(INFO, "Code Complexity: {}", complexity);

                if (dangers.empty()) {
                    LOG_F(INFO, "No potential dangers found.");
                } else {
                    for (const auto& item : dangers) {
                        LOG_F(INFO,
                              "Category: {}\nLine: {}\nCommand: {}\nReason: "
                              "{}\nContext: {}\n",
                              item.category, item.line, item.command,
                              item.reason, item.context.value_or(""));
                    }
                }
                break;
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
                        dangers.emplace_back(
                            DangerItem{category, line, reason, lineNum, {}});
                        detectedIssues.insert(key);
                    }
                }
            }
        }
    }

    static void checkPattern(const std::string& script,
                             const std::regex& pattern,
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

            if (std::regex_search(line, pattern)) {
                std::string key = std::to_string(lineNum) + ":" + category;
                if (!detectedIssues.contains(key)) {
                    dangers.emplace_back(DangerItem{
                        category, line, "Detected usage", lineNum, {}});
                    detectedIssues.insert(key);
                }
            }
        }
    }

    static void checkExternalCommands(
        const std::string& script,
        const std::unordered_set<std::string>& externalCommands,
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

            for (const auto& command : externalCommands) {
                if (line.find(command) != std::string::npos) {
                    std::string key = std::to_string(lineNum) + ":" + command;
                    if (!detectedIssues.contains(key)) {
                        dangers.emplace_back(DangerItem{
                            "External Command",
                            line,
                            "Detected usage of external command: " + command,
                            lineNum,
                            {}});
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
                        dangers.emplace_back(DangerItem{
                            "Suggestion",
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

ScriptAnalyzer::~ScriptAnalyzer() = default;

void ScriptAnalyzer::analyze(const std::string& script, bool output_json,
                             ReportFormat format) {
    impl_->analyze(script, output_json, format);
}

}  // namespace lithium
