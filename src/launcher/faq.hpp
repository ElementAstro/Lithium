#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#include "atom/type/json.hpp"
using json = nlohmann::json;

struct FAQ
{
    std::string question;
    std::string description;
    std::string category;
    std::vector<std::string> solutions;
    int difficulty;
    std::vector<std::string> links;
};

class FAQManager
{
public:
    void addFAQ(const FAQ &faq);

    void deleteFAQ(const std::string &question);

    std::vector<FAQ> searchFAQs(const std::string &keyword);

    std::vector<FAQ> getCategorizedFAQs(const std::string &category);

    void saveToFile(const std::string &filename);

    void loadFromFile(const std::string &filename);

    void printFAQs() const;

private:
    std::vector<FAQ> faqs;
    std::unordered_map<std::string, std::vector<FAQ>> cache;
    std::unordered_map<std::string, std::vector<FAQ>> categoryCache;
    mutable std::mutex mutex;
};