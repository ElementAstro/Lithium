#include "difflib.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>

namespace atom::utils {
static auto joinLines(const std::vector<std::string>& lines) -> std::string {
    std::string joined;
    for (const auto& line : lines) {
        joined += line + "\n";
    }
    return joined;
}

class SequenceMatcher::Impl {
public:
    Impl(std::string str1, std::string str2)
        : seq1_(std::move(str1)), seq2_(std::move(str2)) {
        computeMatchingBlocks();
    }

    void setSeqs(const std::string& str1, const std::string& str2) {
        seq1_ = str1;
        seq2_ = str2;
        computeMatchingBlocks();
    }

    [[nodiscard]] auto ratio() const -> double {
        double matches = sumMatchingBlocks();
        return 2.0 * matches / (seq1_.size() + seq2_.size());
    }

    [[nodiscard]] auto getMatchingBlocks() const
        -> std::vector<std::tuple<int, int, int>> {
        return matching_blocks;
    }

    [[nodiscard]] auto getOpcodes() const
        -> std::vector<std::tuple<std::string, int, int, int, int>> {
        std::vector<std::tuple<std::string, int, int, int, int>> opcodes;
        int aStart = 0;
        int bStart = 0;

        for (const auto& block : matching_blocks) {
            int aIndex = std::get<0>(block);
            int bIndex = std::get<1>(block);
            int size = std::get<2>(block);

            if (size > 0) {
                if (aStart < aIndex || bStart < bIndex) {
                    if (aStart < aIndex && bStart < bIndex) {
                        opcodes.emplace_back("replace", aStart, aIndex, bStart,
                                             bIndex);
                    } else if (aStart < aIndex) {
                        opcodes.emplace_back("delete", aStart, aIndex, bStart,
                                             bStart);
                    } else {
                        opcodes.emplace_back("insert", aStart, aStart, bStart,
                                             bIndex);
                    }
                }
                opcodes.emplace_back("equal", aIndex, aIndex + size, bIndex,
                                     bIndex + size);
                aStart = aIndex + size;
                bStart = bIndex + size;
            }
        }
        return opcodes;
    }

private:
    std::string seq1_;
    std::string seq2_;
    std::vector<std::tuple<int, int, int>> matching_blocks;

    void computeMatchingBlocks() {
        std::unordered_map<char, std::vector<size_t>> seq2_index_map;
        for (size_t j = 0; j < seq2_.size(); ++j) {
            seq2_index_map[seq2_[j]].push_back(j);
        }

        for (size_t i = 0; i < seq1_.size(); ++i) {
            auto it = seq2_index_map.find(seq1_[i]);
            if (it != seq2_index_map.end()) {
                for (size_t j : it->second) {
                    size_t matchLength = 0;
                    while (i + matchLength < seq1_.size() &&
                           j + matchLength < seq2_.size() &&
                           seq1_[i + matchLength] == seq2_[j + matchLength]) {
                        ++matchLength;
                    }
                    if (matchLength > 0) {
                        matching_blocks.emplace_back(i, j, matchLength);
                    }
                }
            }
        }
        matching_blocks.emplace_back(seq1_.size(), seq2_.size(), 0);
        std::sort(matching_blocks.begin(), matching_blocks.end(),
                  [](const std::tuple<int, int, int>& a,
                     const std::tuple<int, int, int>& b) {
                      if (std::get<0>(a) != std::get<0>(b)) {
                          return std::get<0>(a) < std::get<0>(b);
                      }
                      return std::get<1>(a) < std::get<1>(b);
                  });
    }

    [[nodiscard]] auto sumMatchingBlocks() const -> double {
        double matches = 0;
        for (const auto& block : matching_blocks) {
            matches += std::get<2>(block);
        }
        return matches;
    }
};

SequenceMatcher::SequenceMatcher(const std::string& str1,
                                 const std::string& str2)
    : pimpl_(new Impl(str1, str2)) {}
SequenceMatcher::~SequenceMatcher() = default;

void SequenceMatcher::setSeqs(const std::string& str1,
                              const std::string& str2) {
    pimpl_->setSeqs(str1, str2);
}

auto SequenceMatcher::ratio() const -> double { return pimpl_->ratio(); }

auto SequenceMatcher::getMatchingBlocks() const
    -> std::vector<std::tuple<int, int, int>> {
    return pimpl_->getMatchingBlocks();
}

auto SequenceMatcher::getOpcodes() const
    -> std::vector<std::tuple<std::string, int, int, int, int>> {
    return pimpl_->getOpcodes();
}

auto Differ::compare(const std::vector<std::string>& vec1,
                     const std::vector<std::string>& vec2)
    -> std::vector<std::string> {
    std::vector<std::string> result;
    SequenceMatcher matcher("", "");

    size_t i = 0, j = 0;
    while (i < vec1.size() || j < vec2.size()) {
        if (i < vec1.size() && j < vec2.size() && vec1[i] == vec2[j]) {
            result.push_back("  " + vec1[i]);
            ++i;
            ++j;
        } else if (j == vec2.size() ||
                   (i < vec1.size() && (j == 0 || vec1[i] != vec2[j - 1]))) {
            result.push_back("- " + vec1[i]);
            ++i;
        } else {
            result.push_back("+ " + vec2[j]);
            ++j;
        }
    }
    return result;
}

auto Differ::unifiedDiff(const std::vector<std::string>& vec1,
                         const std::vector<std::string>& vec2,
                         const std::string& label1, const std::string& label2,
                         int context) -> std::vector<std::string> {
    std::vector<std::string> diff;
    SequenceMatcher matcher("", "");
    matcher.setSeqs(joinLines(vec1), joinLines(vec2));
    auto opcodes = matcher.getOpcodes();

    diff.push_back("--- " + label1);
    diff.push_back("+++ " + label2);

    int start_a = 0, start_b = 0;
    int end_a = 0, end_b = 0;
    std::vector<std::string> chunk;
    for (const auto& opcode : opcodes) {
        std::string tag = std::get<0>(opcode);
        int i1 = std::get<1>(opcode);
        int i2 = std::get<2>(opcode);
        int j1 = std::get<3>(opcode);
        int j2 = std::get<4>(opcode);

        if (tag == "equal") {
            if (i2 - i1 > 2 * context) {
                chunk.push_back("@@ -" + std::to_string(start_a + 1) + "," +
                                std::to_string(end_a - start_a) + " +" +
                                std::to_string(start_b + 1) + "," +
                                std::to_string(end_b - start_b) + " @@");
                for (int k = start_a;
                     k <
                     std::min(start_a + context, static_cast<int>(vec1.size()));
                     ++k) {
                    chunk.push_back(" " + vec1[k]);
                }
                diff.insert(diff.end(), chunk.begin(), chunk.end());
                chunk.clear();
                start_a = i2 - context;
                start_b = j2 - context;
            } else {
                for (int k = i1; k < i2; ++k) {
                    if (k < vec1.size()) {
                        chunk.push_back(" " + vec1[k]);
                    }
                }
            }
            end_a = i2;
            end_b = j2;
        } else {
            if (chunk.empty()) {
                chunk.push_back("@@ -" + std::to_string(start_a + 1) + "," +
                                std::to_string(end_a - start_a) + " +" +
                                std::to_string(start_b + 1) + "," +
                                std::to_string(end_b - start_b) + " @@");
            }
            if (tag == "replace") {
                for (int k = i1; k < i2; ++k) {
                    if (k < vec1.size()) {
                        chunk.push_back("- " + vec1[k]);
                    }
                }
                for (int k = j1; k < j2; ++k) {
                    if (k < vec2.size()) {
                        chunk.push_back("+ " + vec2[k]);
                    }
                }
            } else if (tag == "delete") {
                for (int k = i1; k < i2; ++k) {
                    if (k < vec1.size()) {
                        chunk.push_back("- " + vec1[k]);
                    }
                }
            } else if (tag == "insert") {
                for (int k = j1; k < j2; ++k) {
                    if (k < vec2.size()) {
                        chunk.push_back("+ " + vec2[k]);
                    }
                }
            }
            end_a = i2;
            end_b = j2;
        }
    }
    if (!chunk.empty()) {
        diff.insert(diff.end(), chunk.begin(), chunk.end());
    }
    return diff;
}

auto HtmlDiff::makeFile(const std::vector<std::string>& fromlines,
                        const std::vector<std::string>& tolines,
                        const std::string& fromdesc,
                        const std::string& todesc) -> std::string {
    std::ostringstream os;
    os << "<html>\n<head><title>Diff</title></head>\n<body>\n";
    os << "<h2>Differences</h2>\n";

    os << "<table border='1'>\n<tr><th>" << fromdesc << "</th><th>" << todesc
       << "</th></tr>\n";

    auto diffs = Differ::compare(fromlines, tolines);
    for (const auto& line : diffs) {
        os << "<tr><td>" << line << "</td></tr>\n";
    }
    os << "</table>\n</body>\n</html>";
    return os.str();
}

auto HtmlDiff::makeTable(const std::vector<std::string>& fromlines,
                         const std::vector<std::string>& tolines,
                         const std::string& fromdesc,
                         const std::string& todesc) -> std::string {
    std::ostringstream os;
    os << "<table border='1'>\n<tr><th>" << fromdesc << "</th><th>" << todesc
       << "</th></tr>\n";

    auto diffs = Differ::compare(fromlines, tolines);
    for (const auto& line : diffs) {
        os << "<tr><td>" << line << "</td></tr>\n";
    }
    os << "</table>\n";
    return os.str();
}

auto getCloseMatches(const std::string& word,
                     const std::vector<std::string>& possibilities, int n,
                     double cutoff) -> std::vector<std::string> {
    std::vector<std::pair<double, std::string>> scores;
    for (const auto& possibility : possibilities) {
        SequenceMatcher matcher(word, possibility);
        double score = matcher.ratio();
        if (score >= cutoff) {
            scores.emplace_back(score, possibility);
        }
    }
    std::sort(scores.begin(), scores.end(),
              [](const std::pair<double, std::string>& a,
                 const std::pair<double, std::string>& b) {
                  return a.first > b.first;
              });
    std::vector<std::string> matches;
    for (int i = 0; i < std::min(n, static_cast<int>(scores.size())); ++i) {
        matches.push_back(scores[i].second);
    }
    return matches;
}
}  // namespace atom::utils
