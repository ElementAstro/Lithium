#include "compiler_output_parser.hpp"

#include <atomic>
#include <fstream>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium {

/**
 * @class HtmlBuilder
 * @brief A simple utility class for building HTML documents.
 *
 * HtmlBuilder provides an interface to construct an HTML document by appending
 * elements like headers, paragraphs, lists, etc. The HTML is stored as a string
 * and can be retrieved using the `str()` method.
 */
class HtmlBuilder {
public:
    HtmlBuilder();

    // Add various HTML elements
    void addTitle(const std::string& title);
    void addHeader(const std::string& header, int level = 1);
    void addParagraph(const std::string& text);
    void addList(const std::vector<std::string>& items);

    // Start and end unordered list
    void startUnorderedList();
    void endUnorderedList();

    // Add list item to an unordered list
    void addListItem(const std::string& item);

    // Get the final HTML document as a string
    auto str() const -> std::string;

private:
    std::ostringstream html_;  ///< String stream to build the HTML document.
    bool inList_{};            ///< Tracks if currently inside a list.
};

HtmlBuilder::HtmlBuilder() {
    html_ << "<html><body>\n";
    LOG_F(INFO, "HtmlBuilder created with initial HTML structure.");
}

void HtmlBuilder::addTitle(const std::string& title) {
    html_ << "<title>" << title << "</title>\n";
    LOG_F(INFO, "Title added: {}", title);
}

void HtmlBuilder::addHeader(const std::string& header, int level) {
    if (level < 1 || level > 6) {
        level = 1;  // Default to <h1> if level is invalid
    }
    html_ << "<h" << level << ">" << header << "</h" << level << ">\n";
    LOG_F(INFO, "Header added: {} at level %d", header, level);
}

void HtmlBuilder::addParagraph(const std::string& text) {
    html_ << "<p>" << text << "</p>\n";
    LOG_F(INFO, "Paragraph added: {}", text);
}

void HtmlBuilder::addList(const std::vector<std::string>& items) {
    html_ << "<ul>\n";
    for (const auto& item : items) {
        html_ << "<li>" << item << "</li>\n";
        LOG_F(INFO, "List item added: {}", item);
    }
    html_ << "</ul>\n";
}

void HtmlBuilder::startUnorderedList() {
    if (!inList_) {
        html_ << "<ul>\n";
        inList_ = true;
        LOG_F(INFO, "Unordered list started.");
    }
}

void HtmlBuilder::endUnorderedList() {
    if (inList_) {
        html_ << "</ul>\n";
        inList_ = false;
        LOG_F(INFO, "Unordered list ended.");
    }
}

void HtmlBuilder::addListItem(const std::string& item) {
    if (inList_) {
        html_ << "<li>" << item << "</li>\n";
        LOG_F(INFO, "List item added inside unordered list: {}", item);
    }
}

std::string HtmlBuilder::str() const {
    std::ostringstream finalHtml;
    finalHtml << html_.str() << "</body></html>\n";
    LOG_F(INFO, "Final HTML document generated.");
    return finalHtml.str();
}

// Message constructor
Message::Message(MessageType t, std::string f, int l, int c, std::string code,
                 std::string func, std::string msg, std::string ctx)
    : type(t),
      file(std::move(f)),
      line(l),
      column(c),
      errorCode(std::move(code)),
      functionName(std::move(func)),
      message(std::move(msg)),
      context(std::move(ctx)) {}

/**
 * @class CompilerOutputParser::Impl
 * @brief The private implementation for CompilerOutputParser (PIMPL idiom).
 */
class CompilerOutputParser::Impl {
public:
    Impl() {
        initRegexPatterns();
        LOG_F(INFO, "CompilerOutputParser::Impl initialized.");
    }

    void parseLine(const std::string& line);
    void parseFile(const std::string& filename);
    void parseFileMultiThreaded(const std::string& filename, int numThreads);
    auto getReport(bool detailed) const -> std::string;
    void generateHtmlReport(const std::string& outputFilename) const;
    auto generateJsonReport() -> json;
    void setCustomRegexPattern(const std::string& compiler,
                               const std::string& pattern);

private:
    std::vector<Message> messages_;
    std::unordered_map<MessageType, std::atomic<int>> counts_;
    mutable std::unordered_map<std::string, std::regex> regexPatterns_;
    mutable std::mutex mutex_;
    std::string currentContext_;
    std::regex includePattern_{R"((.*):(\d+):(\d+):)"};
    std::smatch includeMatch_;

    void initRegexPatterns();
    auto determineType(const std::string& typeStr) const -> MessageType;
    auto toString(MessageType type) const -> std::string;
};

void CompilerOutputParser::Impl::parseLine(const std::string& line) {
    LOG_F(INFO, "Parsing line: {}", line);

    if (std::regex_search(line, includeMatch_, includePattern_)) {
        currentContext_ = includeMatch_.str(1);
        LOG_F(INFO, "Context updated: {}", currentContext_);
        return;
    }

    for (const auto& [compiler, pattern] : regexPatterns_) {
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            MessageType type = determineType(match.str(4));
            std::string file = match.str(1);
            int lineNum = std::stoi(match.str(2));
            int column = match.str(3).empty() ? 0 : std::stoi(match.str(3));
            std::string errorCode = match.size() > 5 ? match.str(5) : "";
            std::string functionName = match.size() > 6 ? match.str(6) : "";
            std::string message =
                match.size() > 7 ? match.str(7) : match.str(5);

            LOG_F(INFO,
                  "Parsed message - File: {}, Line: {}, Column: {}, ErrorCode: "
                  "{}, FunctionName: {}, Message: {}",
                  file, lineNum, column, errorCode, functionName, message);

            std::lock_guard lock(mutex_);
            messages_.emplace_back(type, file, lineNum, column, errorCode,
                                   functionName, message, currentContext_);
            counts_[type]++;
            return;
        }
    }

    std::lock_guard lock(mutex_);
    messages_.emplace_back(MessageType::UNKNOWN, "", 0, 0, "", "", line,
                           currentContext_);
    counts_[MessageType::UNKNOWN]++;
    LOG_F(WARNING, "Unknown message parsed: {}", line);
}

void CompilerOutputParser::Impl::parseFile(const std::string& filename) {
    LOG_F(INFO, "Parsing file: {}", filename);

    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        LOG_F(ERROR, "Failed to open file: {}", filename);
        THROW_FAIL_TO_OPEN_FILE("Failed to open file: " + filename);
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        parseLine(line);
    }

    LOG_F(INFO, "Completed parsing file: {}", filename);
}

void CompilerOutputParser::Impl::parseFileMultiThreaded(
    const std::string& filename, int numThreads) {
    LOG_F(INFO, "Parsing file multithreaded: {} with %d threads", filename,
          numThreads);

    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        LOG_F(ERROR, "Failed to open file: {}", filename);
        THROW_FAIL_TO_OPEN_FILE("Failed to open file: " + filename);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    std::vector<std::jthread> threads;
    auto worker = [this](std::span<const std::string> lines) {
        for (const auto& line : lines) {
            parseLine(line);
        }
    };

    int blockSize = lines.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        auto start = lines.begin() + i * blockSize;
        auto end = (i == numThreads - 1) ? lines.end() : start + blockSize;
        threads.emplace_back(worker, std::span(start, end));
        LOG_F(INFO, "Thread %d started processing lines [{} - {}]", i,
              start - lines.begin(), end - lines.begin());
    }

    // Join the threads once processing is complete
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    LOG_F(INFO, "Multithreaded file parsing completed for file: {}", filename);
}

auto CompilerOutputParser::Impl::getReport(bool detailed) const -> std::string {
    LOG_F(INFO, "Generating report with detailed: {}",
          detailed ? "true" : "false");

    std::ostringstream report;
    report << "Compiler Messages Report:\n";
    report << "Errors: " << counts_.at(MessageType::ERROR) << "\n";
    report << "Warnings: " << counts_.at(MessageType::WARNING) << "\n";
    report << "Notes: " << counts_.at(MessageType::NOTE) << "\n";
    report << "Unknown: " << counts_.at(MessageType::UNKNOWN) << "\n";

    if (detailed) {
        report << "\nDetails:\n";
        for (const auto& msg : messages_) {
            report << "[" << toString(msg.type) << "] ";
            if (!msg.file.empty()) {
                report << msg.file << ":" << msg.line << ":" << msg.column
                       << ": ";
            }
            if (!msg.errorCode.empty()) {
                report << msg.errorCode << " ";
            }
            if (!msg.functionName.empty()) {
                report << msg.functionName << " ";
            }
            report << msg.message << "\n";
            if (!msg.context.empty()) {
                report << "  Context: " << msg.context << "\n";
            }
            for (const auto& note : msg.relatedNotes) {
                report << "  Note: " << note << "\n";
            }
        }
    }

    LOG_F(INFO, "Report generation completed.");
    return report.str();
}

void CompilerOutputParser::Impl::generateHtmlReport(
    const std::string& outputFilename) const {
    LOG_F(INFO, "Generating HTML report: {}", outputFilename);

    HtmlBuilder builder;
    builder.addTitle("Compiler Messages Report");
    builder.addHeader("Compiler Messages Report", 1);

    builder.addHeader("Summary", 2);
    builder.startUnorderedList();
    builder.addListItem("Errors: " +
                        std::to_string(counts_.at(MessageType::ERROR)));
    builder.addListItem("Warnings: " +
                        std::to_string(counts_.at(MessageType::WARNING)));
    builder.addListItem("Notes: " +
                        std::to_string(counts_.at(MessageType::NOTE)));
    builder.addListItem("Unknown: " +
                        std::to_string(counts_.at(MessageType::UNKNOWN)));
    builder.endUnorderedList();

    builder.addHeader("Details", 2);
    builder.startUnorderedList();
    for (const auto& msg : messages_) {
        std::string messageStr = "[" + toString(msg.type) + "] ";
        if (!msg.file.empty()) {
            messageStr += msg.file + ":" + std::to_string(msg.line) + ":" +
                          std::to_string(msg.column) + ": ";
        }
        if (!msg.errorCode.empty()) {
            messageStr += msg.errorCode + " ";
        }
        if (!msg.functionName.empty()) {
            messageStr += msg.functionName + " ";
        }
        messageStr += msg.message;

        builder.addListItem(messageStr);
    }
    builder.endUnorderedList();

    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        LOG_F(ERROR, "Failed to open output file: {}", outputFilename);
        THROW_FAIL_TO_OPEN_FILE("Failed to open output file: " +
                                outputFilename);
    }

    outputFile << builder.str();
    LOG_F(INFO, "HTML report generated and saved to: {}", outputFilename);
}

auto CompilerOutputParser::Impl::generateJsonReport() -> json {
    LOG_F(INFO, "Generating JSON report.");

    json root;
    root["Errors"] = counts_.at(MessageType::ERROR).load();
    root["Warnings"] = counts_.at(MessageType::WARNING).load();
    root["Notes"] = counts_.at(MessageType::NOTE).load();
    root["Unknown"] = counts_.at(MessageType::UNKNOWN).load();

    for (const auto& msg : messages_) {
        json entry;
        entry["Type"] = toString(msg.type);
        entry["File"] = msg.file;
        entry["Line"] = msg.line;
        entry["Column"] = msg.column;
        entry["ErrorCode"] = msg.errorCode;
        entry["FunctionName"] = msg.functionName;
        entry["Message"] = msg.message;
        entry["Context"] = msg.context;
        for (const auto& note : msg.relatedNotes) {
            entry["RelatedNotes"].push_back(note);
        }
        root["Details"].push_back(entry);
    }

    LOG_F(INFO, "JSON report generation completed.");
    return root;
}

void CompilerOutputParser::Impl::setCustomRegexPattern(
    const std::string& compiler, const std::string& pattern) {
    LOG_F(INFO, "Setting custom regex pattern for compiler: {}", compiler);
    std::lock_guard lock(mutex_);
    regexPatterns_[compiler] = std::regex(pattern);
}

void CompilerOutputParser::Impl::initRegexPatterns() {
    LOG_F(INFO, "Initializing regex patterns for supported compilers.");

    regexPatterns_["gcc_clang"] =
        std::regex(R"((.*):(\d+):(\d+): (error|warning|note): (.*))");
    regexPatterns_["msvc"] =
        std::regex(R"((.*)\((\d+),(\d+)\): (error|warning|note) (C\d+): (.*))");
    regexPatterns_["icc"] =
        std::regex(R"((.*)\((\d+)\): (error|remark|warning|note): (.*))");
}

auto CompilerOutputParser::Impl::determineType(const std::string& typeStr) const
    -> MessageType {
    if (typeStr == "error") {
        return MessageType::ERROR;
    }
    if (typeStr == "warning") {
        return MessageType::WARNING;
    }
    if (typeStr == "note" || typeStr == "remark") {
        return MessageType::NOTE;
    }
    return MessageType::UNKNOWN;
}

auto CompilerOutputParser::Impl::toString(MessageType type) const
    -> std::string {
    switch (type) {
        case MessageType::ERROR:
            return "Error";
        case MessageType::WARNING:
            return "Warning";
        case MessageType::NOTE:
            return "Note";
        default:
            return "Unknown";
    }
}

CompilerOutputParser::CompilerOutputParser() : pImpl(std::make_unique<Impl>()) {
    LOG_F(INFO, "CompilerOutputParser created.");
}

CompilerOutputParser::~CompilerOutputParser() = default;

CompilerOutputParser::CompilerOutputParser(CompilerOutputParser&&) noexcept =
    default;
CompilerOutputParser& CompilerOutputParser::operator=(
    CompilerOutputParser&&) noexcept = default;

void CompilerOutputParser::parseLine(const std::string& line) {
    LOG_F(INFO, "Parsing single line.");
    pImpl->parseLine(line);
}

void CompilerOutputParser::parseFile(const std::string& filename) {
    LOG_F(INFO, "Parsing file: {}", filename);
    pImpl->parseFile(filename);
}

void CompilerOutputParser::parseFileMultiThreaded(const std::string& filename,
                                                  int numThreads) {
    LOG_F(INFO, "Parsing file {} with multithreading (%d threads)", filename,
          numThreads);
    pImpl->parseFileMultiThreaded(filename, numThreads);
}

auto CompilerOutputParser::getReport(bool detailed) const -> std::string {
    LOG_F(INFO, "Requesting report with detailed option: {}",
          detailed ? "true" : "false");
    return pImpl->getReport(detailed);
}

void CompilerOutputParser::generateHtmlReport(
    const std::string& outputFilename) const {
    LOG_F(INFO, "Generating HTML report to file: {}", outputFilename);
    pImpl->generateHtmlReport(outputFilename);
}

auto CompilerOutputParser::generateJsonReport() -> json {
    LOG_F(INFO, "Generating JSON report.");
    return pImpl->generateJsonReport();
}

void CompilerOutputParser::setCustomRegexPattern(const std::string& compiler,
                                                 const std::string& pattern) {
    LOG_F(INFO, "Setting custom regex pattern for compiler: {}", compiler);
    pImpl->setCustomRegexPattern(compiler, pattern);
}

}  // namespace lithium
