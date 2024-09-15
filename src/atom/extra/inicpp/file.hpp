#ifndef ATOM_EXTRA_INICPP_INIFILE_HPP
#define ATOM_EXTRA_INICPP_INIFILE_HPP

#include <fstream>
#include <sstream>
#include <vector>
#include "section.hpp"

#include "atom/error/exception.hpp"

namespace inicpp {

template <typename Comparator>
class IniFileBase
    : public std::map<std::string, IniSectionBase<Comparator>, Comparator> {
private:
    char fieldSep_ = '=';
    char esc_ = '\\';
    std::vector<std::string> commentPrefixes_ = {"#", ";"};
    bool multiLineValues_ = false;
    bool overwriteDuplicateFields_ = true;

    void eraseComment(std::string &str, std::string::size_type startpos = 0) {
        for (const auto &commentPrefix : commentPrefixes_) {
            auto pos = str.find(commentPrefix, startpos);
            if (pos != std::string::npos) {
                // Check for escaped comment
                if (pos > 0 && str[pos - 1] == esc_) {
                    str.erase(pos - 1, 1);
                    continue;
                }
                str.erase(pos);
            }
        }
    }

    void writeEscaped(std::ostream &oss, const std::string &str) const {
        for (size_t i = 0; i < str.length(); ++i) {
            auto prefixpos = std::ranges::find_if(
                commentPrefixes_, [&](const std::string &prefix) {
                    return str.find(prefix, i) == i;
                });

            if (prefixpos != commentPrefixes_.end()) {
                oss.put(esc_);
                oss.write(prefixpos->c_str(), prefixpos->size());
                i += prefixpos->size() - 1;
            } else if (multiLineValues_ && str[i] == '\n') {
                oss.write("\n\t", 2);
            } else {
                oss.put(str[i]);
            }
        }
    }

public:
    IniFileBase() = default;

    explicit IniFileBase(const std::string &filename) { load(filename); }

    explicit IniFileBase(std::istream &iss) { decode(iss); }

    ~IniFileBase() = default;

    void setFieldSep(char sep) { fieldSep_ = sep; }

    void setCommentPrefixes(const std::vector<std::string> &commentPrefixes) {
        commentPrefixes_ = commentPrefixes;
    }

    void setEscapeChar(char esc) { esc_ = esc; }

    void setMultiLineValues(bool enable) { multiLineValues_ = enable; }

    void allowOverwriteDuplicateFields(bool allowed) {
        overwriteDuplicateFields_ = allowed;
    }

    /** Decodes a ini file from input stream. */
    void decode(std::istream &iss) {
        this->clear();
        std::string line;
        IniSectionBase<Comparator> *currentSection = nullptr;
        std::string multiLineValueFieldName;

        int lineNo = 0;
        while (std::getline(iss, line)) {
            ++lineNo;
            eraseComment(line);
            bool hasIndent = line.find_first_not_of(indents()) != 0;
            trim(line);

            if (line.empty()) {
                continue;
            }

            if (line.front() == '[') {
                // Section line
                auto pos = line.find(']');
                if (pos == std::string::npos) {
                    THROW_LOGIC_ERROR("Section not closed at line " +
                                      std::to_string(lineNo));
                }
                if (pos == 1) {
                    THROW_LOGIC_ERROR("Empty section name at line " +
                                      std::to_string(lineNo));
                }

                std::string secName = line.substr(1, pos - 1);
                currentSection = &(*this)[secName];
                multiLineValueFieldName.clear();
            } else {
                if (!currentSection)
                    THROW_LOGIC_ERROR("Field without section at line " +
                                      std::to_string(lineNo));

                auto pos = line.find(fieldSep_);
                if (multiLineValues_ && hasIndent &&
                    !multiLineValueFieldName.empty()) {
                    (*currentSection)[multiLineValueFieldName] =
                        (*currentSection)[multiLineValueFieldName]
                            .template as<std::string>() +
                        "\n" + line;
                } else if (pos == std::string::npos) {
                    THROW_LOGIC_ERROR("Field separator missing at line " +
                                      std::to_string(lineNo));
                } else {
                    std::string name = line.substr(0, pos);
                    trim(name);

                    if (!overwriteDuplicateFields_ &&
                        currentSection->count(name)) {
                        THROW_LOGIC_ERROR("Duplicate field at line " +
                                          std::to_string(lineNo));
                    }

                    std::string value = line.substr(pos + 1);
                    trim(value);
                    (*currentSection)[name] = value;

                    multiLineValueFieldName = name;
                }
            }
        }
    }

    /** Decodes an ini file from a string. */
    void decode(const std::string &content) {
        std::istringstream ss(content);
        decode(ss);
    }

    /** Loads and decodes an ini file from a file path. */
    void load(const std::string &fileName) {
        std::ifstream iss(fileName);
        if (!iss.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Unable to open file " + fileName);
        }
        decode(iss);
    }

    /** Encodes the ini file to the output stream. */
    void encode(std::ostream &oss) const {
        for (const auto &sectionPair : *this) {
            oss << '[' << sectionPair.first << "]\n";
            for (const auto &fieldPair : sectionPair.second) {
                oss << fieldPair.first << fieldSep_
                    << fieldPair.second.template as<std::string>() << "\n";
            }
        }
    }

    /** Encodes the ini file to a string and returns it. */
    [[nodiscard]] auto encode() const -> std::string {
        std::ostringstream sss;
        encode(sss);
        return sss.str();
    }

    /** Saves the ini file to a given file path. */
    void save(const std::string &fileName) const {
        std::ofstream oss(fileName);
        if (!oss.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Unable to open file " + fileName);
        }
        encode(oss);
    }
};

using IniFile = IniFileBase<std::less<>>;
using IniFileCaseInsensitive = IniFileBase<StringInsensitiveLess>;

}  // namespace inicpp

#endif  // ATOM_EXTRA_INICPP_INIFILE_HPP