#include "analysts.hpp"

#include <fstream>
#include <thread>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"

namespace lithium {
void CompilerOutputParser::parseLine(const std::string& line) {
    if (std::regex_search(line, includeMatch_, includePattern_)) {
        currentContext_ = includeMatch_.str(1);
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
}

void CompilerOutputParser::parseFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open file: " + filename);
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        parseLine(line);
    }
}

void CompilerOutputParser::parseFileMultiThreaded(const std::string& filename,
                                                  int numThreads) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open file: " + filename);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    std::vector<std::thread> threads;
    auto worker = [this](const std::vector<std::string>& lines, int start,
                         int end) {
        for (int i = start; i < end; ++i) {
            parseLine(lines[i]);
        }
    };

    int blockSize = lines.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * blockSize;
        int end = (i == numThreads - 1) ? lines.size() : (i + 1) * blockSize;
        threads.emplace_back(worker, std::cref(lines), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

auto CompilerOutputParser::getReport(bool detailed) const -> std::string {
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

    return report.str();
}

void CompilerOutputParser::generateHtmlReport(
    const std::string& outputFilename) const {
    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open output file: " +
                                outputFilename);
    }

    outputFile << "<html><body>\n";
    outputFile << "<h1>Compiler Messages Report</h1>\n";
    outputFile << "<ul>\n";
    outputFile << "<li>Errors: " << counts_.at(MessageType::ERROR) << "</li>\n";
    outputFile << "<li>Warnings: " << counts_.at(MessageType::WARNING)
               << "</li>\n";
    outputFile << "<li>Notes: " << counts_.at(MessageType::NOTE) << "</li>\n";
    outputFile << "<li>Unknown: " << counts_.at(MessageType::UNKNOWN)
               << "</li>\n";
    outputFile << "</ul>\n";

    outputFile << "<h2>Details</h2>\n";
    outputFile << "<ul>\n";
    for (const auto& msg : messages_) {
        outputFile << "<li><b>[" << toString(msg.type) << "]</b> ";
        if (!msg.file.empty()) {
            outputFile << msg.file << ":" << msg.line << ":" << msg.column
                       << ": ";
        }
        if (!msg.errorCode.empty()) {
            outputFile << msg.errorCode << " ";
        }
        if (!msg.functionName.empty()) {
            outputFile << msg.functionName << " ";
        }
        outputFile << msg.message << "</li>\n";
        if (!msg.context.empty()) {
            outputFile << "<li>Context: " << msg.context << "</li>\n";
        }
        for (const auto& note : msg.relatedNotes) {
            outputFile << "<li>Note: " << note << "</li>\n";
        }
    }
    outputFile << "</ul>\n";
    outputFile << "</body></html>\n";
}

auto CompilerOutputParser::generateJsonReport() -> json {
    json root;
    root["Errors"] = counts_[MessageType::ERROR];
    root["Warnings"] = counts_[MessageType::WARNING];
    root["Notes"] = counts_[MessageType::NOTE];
    root["Unknown"] = counts_[MessageType::UNKNOWN];

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

    return root;
}

void CompilerOutputParser::setCustomRegexPattern(const std::string& compiler,
                                                 const std::string& pattern) {
    std::lock_guard lock(mutex_);
    regexPatterns_[compiler] = std::regex(pattern);
}

void CompilerOutputParser::initRegexPatterns() {
    regexPatterns_["gcc_clang"] =
        std::regex(R"((.*):(\d+):(\d+): (error|warning|note): (.*))");
    regexPatterns_["msvc"] =
        std::regex(R"((.*)\((\d+),(\d+)\): (error|warning|note) (C\d+): (.*))");
    regexPatterns_["icc"] =
        std::regex(R"((.*)\((\d+)\): (error|remark|warning|note): (.*))");
}

auto CompilerOutputParser::determineType(const std::string& typeStr) const
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

auto CompilerOutputParser::toString(MessageType type) const -> std::string {
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

}  // namespace lithium
