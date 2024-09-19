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
#pragma unroll
            for (size_t index = 0; index < row.size(); ++index) {
                widths[index] = std::max(widths[index], row[index].size());
            }
        }
    }

    if (showHeader) {
        printRow(data[0], widths);
        printSeparator(widths);
    }

#pragma unroll
    for (size_t rowIndex = (showHeader ? 1 : 0); rowIndex < data.size();
         ++rowIndex) {
        printRow(data[rowIndex], widths);
    }
}

void TableOutputStyle::printRow(const std::vector<std::string>& row,
                                const std::vector<size_t>& widths) const {
#pragma unroll
    for (size_t index = 0; index < row.size(); ++index) {
        std::string content = row[index];
        size_t width = widths[index];
        std::string alignedContent;

        switch (columnAlignments[index]) {
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
#pragma unroll
    for (size_t width : widths) {
        std::cout << std::string(width, '-') << "-+-";
    }
    std::cout << '\n';
}

void CSVOutputStyle::print(
    const std::vector<std::vector<std::string>>& data) const {
#pragma unroll
    for (const auto& row : data) {
#pragma unroll
        for (size_t index = 0; index < row.size(); ++index) {
            std::cout << row[index];
            if (index < row.size() - 1) {
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
#pragma unroll
    for (size_t rowIndex = 0; rowIndex < data.size(); ++rowIndex) {
        std::cout << "  {";
#pragma unroll
        for (size_t colIndex = 0; colIndex < data[rowIndex].size();
             ++colIndex) {
            std::cout << "\"" << (rowIndex == 0 ? "header" : "data") << colIndex
                      << "\": \"" << data[rowIndex][colIndex] << "\"";
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

DataPrinter::DataPrinter(std::unique_ptr<OutputStyle> style)
    : style_(std::move(style)) {}

void DataPrinter::print(
    const std::vector<std::vector<std::string>>& data) const {
    style_->print(data);
}
