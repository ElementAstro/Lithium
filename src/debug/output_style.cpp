#include "output_style.hpp"

#include <algorithm>
#include <format>
#include <iostream>
#include <sstream>

//////////////////////
// TableOutputStyle //
//////////////////////

class TableOutputStyle::Impl {
public:
    Impl(bool header) : showHeader(header) {}

    void setColumnWidths(const std::vector<size_t>& widths) {
        colWidths = widths;
    }

    void setAlignment(const std::vector<Alignment>& alignments) {
        columnAlignments = alignments;
    }

    void print(const std::vector<std::vector<std::string>>& data) const {
        if (data.empty()) {
            return;
        }

        std::vector<size_t> widths = colWidths;
        if (widths.empty()) {
            widths.resize(data[0].size(), 0);
            for (const auto& row : data) {
                for (size_t index = 0; index < row.size(); ++index) {
                    widths[index] = std::max(widths[index], row[index].size());
                }
            }
        }

        if (showHeader) {
            printRow(data[0], widths);
            printSeparator(widths);
        }

        for (size_t rowIndex = (showHeader ? 1 : 0); rowIndex < data.size();
             ++rowIndex) {
            printRow(data[rowIndex], widths);
        }
    }

private:
    bool showHeader;
    std::vector<size_t> colWidths;
    std::vector<Alignment> columnAlignments;

    void printRow(const std::vector<std::string>& row,
                  const std::vector<size_t>& widths) const {
        for (size_t index = 0; index < row.size(); ++index) {
            std::string content = row[index];
            size_t width = widths[index];
            std::string alignedContent;

            if (index < columnAlignments.size()) {
                switch (columnAlignments[index]) {
                    case Alignment::LEFT:
                        alignedContent = std::format("{:<{}}", content, width);
                        break;
                    case Alignment::RIGHT:
                        alignedContent = std::format("{:>{}}", content, width);
                        break;
                    case Alignment::CENTER: {
                        size_t padding = (width > content.size())
                                             ? (width - content.size()) / 2
                                             : 0;
                        alignedContent =
                            std::string(padding, ' ') + content +
                            std::string(width - padding - content.size(), ' ');
                        break;
                    }
                }
            } else {
                alignedContent = content;
            }
            std::cout << alignedContent << " | ";
        }
        std::cout << '\n';
    }

    void printSeparator(const std::vector<size_t>& widths) const {
        for (size_t width : widths) {
            std::cout << std::string(width, '-') << "-+-";
        }
        std::cout << '\n';
    }
};

TableOutputStyle::TableOutputStyle(bool header)
    : impl_(std::make_unique<Impl>(header)) {}

void TableOutputStyle::setColumnWidths(const std::vector<size_t>& widths) {
    impl_->setColumnWidths(widths);
}

void TableOutputStyle::setAlignment(const std::vector<Alignment>& alignments) {
    impl_->setAlignment(alignments);
}

void TableOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    impl_->print(data);
}

/////////////////////
// CSVOutputStyle //
/////////////////////

class CSVOutputStyle::Impl {
public:
    void print(const std::vector<std::vector<std::string>>& data) const {
        for (const auto& row : data) {
            for (size_t index = 0; index < row.size(); ++index) {
                std::cout << '"' << escapeCSV(row[index]) << '"';
                if (index < row.size() - 1) {
                    std::cout << ",";
                }
            }
            std::cout << '\n';
        }
    }

private:
    std::string escapeCSV(const std::string& field) const {
        std::string escaped = field;
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos) {
            escaped.insert(pos, "\"");
            pos += 2;
        }
        return escaped;
    }
};

CSVOutputStyle::CSVOutputStyle() : impl_(std::make_unique<Impl>()) {}

void CSVOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    impl_->print(data);
}

//////////////////////
// JSONOutputStyle //
//////////////////////

class JSONOutputStyle::Impl {
public:
    void print(const std::vector<std::vector<std::string>>& data) const {
        if (data.empty()) {
            std::cout << "[]\n";
            return;
        }

        std::cout << "[\n";
        for (size_t rowIndex = 0; rowIndex < data.size(); ++rowIndex) {
            std::cout << "  {";
            for (size_t colIndex = 0; colIndex < data[rowIndex].size();
                 ++colIndex) {
                std::cout << "\"column" << colIndex << "\": \""
                          << escapeJSON(data[rowIndex][colIndex]) << "\"";
                if (colIndex < data[rowIndex].size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "}";
            if (rowIndex < data.size() - 1) {
                std::cout << ",";
            }
            std::cout << '\n';
        }
        std::cout << "]\n";
    }

private:
    std::string escapeJSON(const std::string& s) const {
        std::ostringstream oss;
        for (char c : s) {
            switch (c) {
                case '\"':
                    oss << "\\\"";
                    break;
                case '\\':
                    oss << "\\\\";
                    break;
                case '\b':
                    oss << "\\b";
                    break;
                case '\f':
                    oss << "\\f";
                    break;
                case '\n':
                    oss << "\\n";
                    break;
                case '\r':
                    oss << "\\r";
                    break;
                case '\t':
                    oss << "\\t";
                    break;
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        oss << "\\u" << std::hex << std::uppercase << (int)c;
                    } else {
                        oss << c;
                    }
            }
        }
        return oss.str();
    }
};

JSONOutputStyle::JSONOutputStyle() : impl_(std::make_unique<Impl>()) {}

void JSONOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    impl_->print(data);
}

/////////////////////
// XMLOutputStyle //
/////////////////////

class XMLOutputStyle::Impl {
public:
    void print(const std::vector<std::vector<std::string>>& data) const {
        if (data.empty()) {
            std::cout << "<data></data>\n";
            return;
        }

        std::cout << "<data>\n";
        for (const auto& row : data) {
            std::cout << "  <record>\n";
            for (size_t colIndex = 0; colIndex < row.size(); ++colIndex) {
                std::cout << "    <column" << colIndex << ">"
                          << escapeXML(row[colIndex]) << "</column" << colIndex
                          << ">\n";
            }
            std::cout << "  </record>\n";
        }
        std::cout << "</data>\n";
    }

private:
    std::string escapeXML(const std::string& s) const {
        std::string escaped;
        for (char c : s) {
            switch (c) {
                case '&':
                    escaped += "&amp;";
                    break;
                case '\"':
                    escaped += "&quot;";
                    break;
                case '\'':
                    escaped += "&apos;";
                    break;
                case '<':
                    escaped += "&lt;";
                    break;
                case '>':
                    escaped += "&gt;";
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        return escaped;
    }
};

XMLOutputStyle::XMLOutputStyle() : impl_(std::make_unique<Impl>()) {}

void XMLOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    impl_->print(data);
}

//////////////////////////
// MarkdownOutputStyle //
//////////////////////////

class MarkdownOutputStyle::Impl {
public:
    void print(const std::vector<std::vector<std::string>>& data) const {
        if (data.empty()) {
            return;
        }

        // Header
        printRow(data[0], true);
        // Separator
        printSeparator(data[0].size());
        // Data rows
        for (size_t rowIndex = 1; rowIndex < data.size(); ++rowIndex) {
            printRow(data[rowIndex], false);
        }
    }

private:
    void printRow(const std::vector<std::string>& row, bool isHeader) const {
        std::cout << "|";
        for (const auto& cell : row) {
            std::cout << " " << cell << " |";
        }
        std::cout << '\n';
    }

    void printSeparator(size_t numColumns) const {
        std::cout << "|";
        for (size_t i = 0; i < numColumns; ++i) {
            std::cout << " --- |";
        }
        std::cout << '\n';
    }
};

MarkdownOutputStyle::MarkdownOutputStyle() : impl_(std::make_unique<Impl>()) {}

void MarkdownOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    impl_->print(data);
}

/////////////////////
// DataPrinter Impl //
/////////////////////

class DataPrinter::Impl {
public:
    Impl(std::unique_ptr<OutputStyle> style) : style_(std::move(style)) {}

    void print(const std::vector<std::vector<std::string>>& data) const {
        if (style_) {
            style_->print(data);
        }
    }

private:
    std::unique_ptr<OutputStyle> style_;
};

DataPrinter::DataPrinter(std::unique_ptr<OutputStyle> style)
    : impl_(std::make_unique<Impl>(std::move(style))) {}

void DataPrinter::print(
    const std::vector<std::vector<std::string>>& data) const {
    impl_->print(data);
}