#include "output_style.hpp"

#include <format>
#include <iostream>

TableOutputStyle::TableOutputStyle(bool header) : showHeader(header) {}

void TableOutputStyle::setColumnWidths(const std::vector<size_t>& widths) {
    colWidths = widths;
}

void TableOutputStyle::setAlignment(const std::vector<Alignment>& alignments) {
    columnAlignments = alignments;
}

void TableOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    if (data.empty()) {
        return;
    }

    std::vector<size_t> widths = colWidths;
    if (widths.empty()) {
        widths.resize(data[0].size(), 0);
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                widths[i] = std::max(widths[i], row[i].size());
            }
        }
    }

    if (showHeader) {
        printRow(data[0], widths);
        printSeparator(widths);
    }

    for (size_t r = (showHeader ? 1 : 0); r < data.size(); ++r) {
        printRow(data[r], widths);
    }
}

void TableOutputStyle::printRow(const std::vector<std::string>& row,
                                const std::vector<size_t>& widths) const {
    for (size_t i = 0; i < row.size(); ++i) {
        std::string content = row[i];
        size_t width = widths[i];
        std::string alignedContent;

        switch (columnAlignments[i]) {
            case Alignment::LEFT:
                alignedContent = std::format("{:<{}}", content, width);
                break;
            case Alignment::RIGHT:
                alignedContent = std::format("{:>{}}", content, width);
                break;
            case Alignment::CENTER: {
                size_t padding = (width - content.size()) / 2;
                alignedContent =
                    std::string(padding, ' ') + content +
                    std::string(width - padding - content.size(), ' ');
                break;
            }
        }
        std::cout << alignedContent << " | ";
    }
    std::cout << '\n';
}

void TableOutputStyle::printSeparator(const std::vector<size_t>& widths) const {
    for (size_t width : widths) {
        std::cout << std::string(width, '-') << "-+-";
    }
    std::cout << '\n';
}

void CSVOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i];
            if (i < row.size() - 1) {
                std::cout << ",";
            }
        }
        std::cout << '\n';
    }
}

void JSONOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
    if (data.empty()) {
        return;
    }

    std::cout << "[\n";
    for (size_t r = 0; r < data.size(); ++r) {
        std::cout << "  {";
        for (size_t c = 0; c < data[r].size(); ++c) {
            std::cout << "\"" << (r == 0 ? "header" : "data") << c << "\": \""
                      << data[r][c] << "\"";
            if (c < data[r].size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "}";
        if (r < data.size() - 1) {
            std::cout << ",";
        }
        std::cout << '\n';
    }
    std::cout << "]\n";
}

DataPrinter::DataPrinter(std::unique_ptr<OutputStyle> style)
    : style_(std::move(style)) {}

void DataPrinter::print(
    const std::vector<std::vector<std::string>>& data) const {
    style_->print(data);
}
