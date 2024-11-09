#ifndef STAR_SEARCH_SEARCH_HPP
#define STAR_SEARCH_SEARCH_HPP

#include <memory>
#include <string>
#include <vector>

namespace lithium::target {

/**
 * @brief Represents a star object with a name, aliases, and a click count.
 */
struct StarObject {
private:
    std::string name_;
    std::vector<std::string> aliases_;
    int clickCount_;

public:
    StarObject(std::string name, std::initializer_list<std::string> aliases,
               int clickCount = 0)
        : name_(std::move(name)), aliases_(aliases), clickCount_(clickCount) {}

    // Accessor methods
    [[nodiscard]] const std::string& getName() const { return name_; }
    [[nodiscard]] const std::vector<std::string>& getAliases() const {
        return aliases_;
    }
    [[nodiscard]] int getClickCount() const { return clickCount_; }

    // Mutator methods
    void setName(const std::string& name) { name_ = name; }
    void setAliases(const std::vector<std::string>& aliases) {
        aliases_ = aliases;
    }
    void setClickCount(int clickCount) { clickCount_ = clickCount; }
};

/**
 * @brief A search engine for star objects.
 */
class SearchEngine {
public:
    SearchEngine();
    ~SearchEngine();

    void addStarObject(const StarObject& starObject);
    std::vector<StarObject> searchStarObject(const std::string& query) const;
    std::vector<StarObject> fuzzySearchStarObject(const std::string& query,
                                                  int tolerance) const;
    std::vector<std::string> autoCompleteStarObject(
        const std::string& prefix) const;
    static std::vector<StarObject> getRankedResults(
        std::vector<StarObject>& results);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace lithium::target

#endif