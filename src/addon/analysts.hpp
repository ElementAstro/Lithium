#ifndef LITHIUM_ADDON_COMPILER_ANALYSIS_HPP
#define LITHIUM_ADDON_COMPILER_ANALYSIS_HPP

#include <memory>
#include <string>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

#include "macro.hpp"

namespace lithium {

/**
 * @enum MessageType
 * @brief Represents the type of a compiler message.
 */
enum class MessageType { ERROR, WARNING, NOTE, UNKNOWN };

/**
 * @struct Message
 * @brief Holds information about a single compiler message.
 */
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
} ATOM_ALIGNAS(128);

/**
 * @class CompilerOutputParser
 * @brief Parses compiler output and generates reports.
 *
 * Uses regular expressions to parse compiler messages from various compilers,
 * supports both single-threaded and multi-threaded parsing.
 */
class CompilerOutputParser {
public:
    CompilerOutputParser();
    ~CompilerOutputParser();

    // Delete copy constructor and copy assignment to avoid copying state
    CompilerOutputParser(const CompilerOutputParser&) = delete;
    CompilerOutputParser& operator=(const CompilerOutputParser&) = delete;

    // Enable move semantics
    CompilerOutputParser(CompilerOutputParser&&) noexcept;
    CompilerOutputParser& operator=(CompilerOutputParser&&) noexcept;

    void parseLine(const std::string& line);
    void parseFile(const std::string& filename);
    void parseFileMultiThreaded(const std::string& filename, int numThreads);
    auto getReport(bool detailed = true) const -> std::string;
    void generateHtmlReport(const std::string& outputFilename) const;
    auto generateJsonReport() -> json;
    void setCustomRegexPattern(const std::string& compiler,
                               const std::string& pattern);

private:
    class Impl;                   // Forward declaration of implementation class
    std::unique_ptr<Impl> pImpl;  // PIMPL idiom, pointer to implementation
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_COMPILER_ANALYSIS_HPP
