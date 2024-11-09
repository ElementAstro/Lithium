#include "reader.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"
#include "atom/utils/utf.hpp"

namespace lithium::target {
// Dialect 构造函数实现
Dialect::Dialect(char delim, char quote, bool dquote, bool skipspace,
                 std::string lineterm, Quoting quote_mode)
    : delimiter(delim),
      quotechar(quote),
      doublequote(dquote),
      skip_initial_space(skipspace),
      lineterminator(std::move(lineterm)),
      quoting(quote_mode) {}

// DictReader 实现
class DictReader::Impl {
public:
    Impl(std::istream& input, const std::vector<std::string>& fieldnames,
         Dialect dialect, Encoding encoding)
        : dialect_(std::move(dialect)),
          fieldnames_(fieldnames),
          input_(input),
          encoding_(encoding),
          delimiter_(dialect_.delimiter) {  // 初始化 delimiter_
        if (fieldnames_.empty()) {
            throw std::invalid_argument("字段名不能为空。");
        }
        if (!detectDialect(input)) {
            throw std::runtime_error("方言检测失败。");
        }

        // 如果提供了字段名，跳过第一行头部
        if (!fieldnames_.empty()) {
            std::getline(input_, current_line_, '\n');
            if (encoding_ == Encoding::UTF16) {
                std::u16string u16CurrentLine(current_line_.begin(),
                                              current_line_.end());
                current_line_ = atom::utils::utf16toUtF8(u16CurrentLine);
            }
        }
    }

    auto next(std::unordered_map<std::string, std::string>& row) -> bool {
        if (!std::getline(input_, current_line_, '\n')) {
            return false;
        }

        if (encoding_ == Encoding::UTF16) {
            std::u16string u16CurrentLine(current_line_.begin(),
                                          current_line_.end());
            current_line_ = atom::utils::utf16toUtF8(u16CurrentLine);
        }

        std::vector<std::string> parsedLine = parseLine(current_line_);
        row.clear();

        for (size_t i = 0; i < fieldnames_.size(); ++i) {
            if (i < parsedLine.size()) {
                row[fieldnames_[i]] = parsedLine[i];
            } else {
                row[fieldnames_[i]] = "";
            }
        }
        return true;
    }

private:
    auto detectDialect(std::istream& input) -> bool {
        // 简单检测分隔符和引用字符
        std::string line;
        if (std::getline(input, line)) {
            size_t comma = std::count(line.begin(), line.end(), ',');
            size_t semicolon = std::count(line.begin(), line.end(), ';');
            delimiter_ = (semicolon > comma) ? ';' : ',';
            dialect_.delimiter = delimiter_;
            // 检测是否使用引号
            size_t quoteCount =
                std::count(line.begin(), line.end(), dialect_.quotechar);
            dialect_.quoting = (quoteCount > 0) ? Quoting::ALL : Quoting::NONE;
            // 重置流
            input.clear();
            input.seekg(0, std::ios::beg);
            return true;
        }
        return false;
    }

    [[nodiscard]] auto parseLine(const std::string& line) const -> std::vector<std::string> {
        std::vector<std::string> result;
        std::string cell;
        bool insideQuotes = false;

        for (char ch : line) {
            if (ch == dialect_.quotechar) {
                if (dialect_.doublequote) {
                    if (insideQuotes && !cell.empty() &&
                        cell.back() == dialect_.quotechar) {
                        cell.pop_back();
                        cell += ch;
                        continue;
                    }
                }
                insideQuotes = !insideQuotes;
            } else if (ch == dialect_.delimiter && !insideQuotes) {
                result.push_back(atom::utils::trim(cell));
                cell.clear();
            } else {
                cell += ch;
            }
        }
        result.push_back(atom::utils::trim(cell));
        return result;
    }

    Dialect dialect_;
    std::vector<std::string> fieldnames_;
    std::istream& input_;
    std::string current_line_;
    Encoding encoding_;
    char delimiter_;
};

DictReader::DictReader(std::istream& input,
                       const std::vector<std::string>& fieldnames,
                       Dialect dialect, Encoding encoding)
    : pimpl_(std::make_unique<Impl>(input, fieldnames, std::move(dialect),
                                    encoding)) {}

bool DictReader::next(std::unordered_map<std::string, std::string>& row) {
    return pimpl_->next(row);
}

// DictWriter 实现
class DictWriter::Impl {
public:
    Impl(std::ostream& output, const std::vector<std::string>& fieldnames,
         Dialect dialect, bool quote_all, Encoding encoding)
        : dialect_(std::move(dialect)),
          fieldnames_(fieldnames),
          output_(output),
          quote_all_(quote_all),
          encoding_(encoding) {
        writeHeader();
    }

    void writeRow(const std::unordered_map<std::string, std::string>& row) {
        std::vector<std::string> outputRow;
        for (const auto& fieldname : fieldnames_) {
            if (row.find(fieldname) != row.end()) {
                outputRow.push_back(escape(row.at(fieldname)));
            } else {
                outputRow.emplace_back("");
            }
        }
        writeLine(outputRow);
    }

private:
    void writeHeader() { writeLine(fieldnames_); }

    void writeLine(const std::vector<std::string>& line) {
        for (size_t i = 0; i < line.size(); ++i) {
            if (i > 0) {
                output_ << dialect_.delimiter;
            }
            if (encoding_ == Encoding::UTF16) {
                std::u16string field = atom::utils::utf8toUtF16(line[i]);
                if (quote_all_ || needsQuotes(line[i])) {
                    field.insert(field.begin(), dialect_.quotechar);
                    field.push_back(dialect_.quotechar);
                }
                output_ << atom::utils::utf16toUtF8(field);
            } else {
                std::string field = line[i];
                if (quote_all_ || needsQuotes(field)) {
                    field.insert(field.begin(), dialect_.quotechar);
                    field.push_back(dialect_.quotechar);
                }
                output_ << field;
            }
        }
        output_ << dialect_.lineterminator;
    }

    [[nodiscard]] auto needsQuotes(const std::string& field) const -> bool {
        return field.find(dialect_.delimiter) != std::string::npos ||
               field.find(dialect_.quotechar) != std::string::npos ||
               field.find('\n') != std::string::npos;
    }

    [[nodiscard]] auto escape(const std::string& field) const -> std::string {
        if (dialect_.quoting == Quoting::ALL || needsQuotes(field)) {
            std::string escaped = field;
            if (dialect_.doublequote) {
                size_t pos = 0;
                while ((pos = escaped.find(dialect_.quotechar, pos)) !=
                       std::string::npos) {
                    escaped.insert(pos, 1, dialect_.quotechar);
                    pos += 2;
                }
            }
            return escaped;
        }
        return field;
    }

    Dialect dialect_;
    std::vector<std::string> fieldnames_;
    std::ostream& output_;
    bool quote_all_;
    Encoding encoding_;
};

DictWriter::DictWriter(std::ostream& output,
                       const std::vector<std::string>& fieldnames,
                       Dialect dialect, bool quote_all, Encoding encoding)
    : pimpl_(std::make_unique<Impl>(output, fieldnames, std::move(dialect),
                                    quote_all, encoding)) {}

void DictWriter::writeRow(
    const std::unordered_map<std::string, std::string>& row) {
    pimpl_->writeRow(row);
}
}  // namespace lithium::target