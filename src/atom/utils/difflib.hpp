#ifndef ATOM_UTILS_DIFFLIB_HPP
#define ATOM_UTILS_DIFFLIB_HPP

#include <memory>
#include <string>
#include <vector>

namespace atom::utils {
class SequenceMatcher {
public:
    SequenceMatcher(const std::string& str1, const std::string& str2);
    ~SequenceMatcher();

    void setSeqs(const std::string& str1, const std::string& str2);
    [[nodiscard]] auto ratio() const -> double;
    [[nodiscard]] auto getMatchingBlocks() const
        -> std::vector<std::tuple<int, int, int>>;
    [[nodiscard]] auto getOpcodes() const
        -> std::vector<std::tuple<std::string, int, int, int, int>>;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

class Differ {
public:
    static auto compare(const std::vector<std::string>& vec1,
                        const std::vector<std::string>& vec2)
        -> std::vector<std::string>;
    static auto unifiedDiff(const std::vector<std::string>& vec1,
                            const std::vector<std::string>& vec2,
                            const std::string& label1 = "a",
                            const std::string& label2 = "b",
                            int context = 3) -> std::vector<std::string>;
};

class HtmlDiff {
public:
    static auto makeFile(const std::vector<std::string>& fromlines,
                         const std::vector<std::string>& tolines,
                         const std::string& fromdesc = "",
                         const std::string& todesc = "") -> std::string;
    static auto makeTable(const std::vector<std::string>& fromlines,
                          const std::vector<std::string>& tolines,
                          const std::string& fromdesc = "",
                          const std::string& todesc = "") -> std::string;
};

auto getCloseMatches(const std::string& word,
                     const std::vector<std::string>& possibilities, int n = 3,
                     double cutoff = 0.6) -> std::vector<std::string>;
}  // namespace atom::utils

#endif  // ATOM_UTILS_DIFFLIB_HPP
