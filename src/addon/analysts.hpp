#ifndef LITHIUM_ADDON_COMPILER_ANALYSIS_HPP
#define LITHIUM_ADDON_COMPILER_ANALYSIS_HPP

#include <mutex>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/type/json_fwd.hpp"

namespace lithium {

using json = nlohmann::json;

enum class MessageType { ERROR, WARNING, NOTE, UNKNOWN };

struct Message {
    MessageType type;
    std::string file;
    int line;
    int column;
    std::string errorCode;
    std::string functionName;
    std::string message;
    std::string context;
    std::vector<std::string> relatedNotes;

    Message(MessageType t, std::string f, int l, int c, std::string code,
            std::string func, std::string msg, std::string ctx);
};

class CompilerOutputParser {
public:
    CompilerOutputParser();

    void parseLine(const std::string& line);
    void parseFile(const std::string& filename);
    void parseFileMultiThreaded(const std::string& filename, int numThreads);
    auto getReport(bool detailed = true) const -> std::string;
    void generateHtmlReport(const std::string& outputFilename) const;
    auto generateJsonReport() -> json;
    void setCustomRegexPattern(const std::string& compiler,
                               const std::string& pattern);

private:
    std::vector<Message> messages_;
    std::unordered_map<MessageType, int> counts_;
    mutable std::unordered_map<std::string, std::regex> regexPatterns_;
    mutable std::mutex mutex_;
    std::string currentContext_;
    std::regex includePattern_;
    std::smatch includeMatch_;

    void initRegexPatterns();
    MessageType determineType(const std::string& typeStr) const;
    std::string toString(MessageType type) const;
};

}  // namespace lithium

#endif
