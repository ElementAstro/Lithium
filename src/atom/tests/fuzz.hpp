#ifndef ATOM_TESTS_FUZZ_HPP
#define ATOM_TESTS_FUZZ_HPP

#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#undef CHAR_MIN
#undef CHAR_MAX

class RandomDataGenerator {
private:
    static constexpr int DEFAULT_INT_MAX = 100;

    static constexpr int CHAR_MIN = 32;
    static constexpr int CHAR_MAX = 126;
    static constexpr int IPV4_SEGMENT_MAX = 256;
    static constexpr int MAC_SEGMENTS = 6;
    static constexpr int MAC_SEGMENT_MAX = 256;
    static constexpr int URL_DOMAIN_LENGTH = 8;
    static constexpr int FILE_PATH_SEGMENT_LENGTH = 5;
    static constexpr int FILE_PATH_EXTENSION_LENGTH = 3;
    static constexpr int JSON_PRECISION = 6;

    std::mt19937 generator_;
    std::uniform_int_distribution<> intDistribution_;
    std::uniform_real_distribution<> realDistribution_;
    std::uniform_int_distribution<> charDistribution_;

public:
    explicit RandomDataGenerator(int seed = std::random_device{}());

    auto generateIntegers(int count, int min = 0,
                          int max = DEFAULT_INT_MAX) -> std::vector<int>;

    auto generateReals(int count, double min = 0.0,
                       double max = 1.0) -> std::vector<double>;

    auto generateString(int length, bool alphanumeric = false) -> std::string;

    auto generateBooleans(int count) -> std::vector<bool>;

    auto generateException() -> std::string;

    auto generateDateTime(const std::chrono::system_clock::time_point& start,
                          const std::chrono::system_clock::time_point& end)
        -> std::chrono::system_clock::time_point;

    auto generateRegexMatch(const std::string& regexStr) -> std::string;

    auto generateFilePath(const std::string& baseDir,
                          int depth = 3) -> std::filesystem::path;

    auto generateRandomJSON(int depth = 2) -> std::string;

    auto generateRandomXML(int depth = 2) -> std::string;

    template <typename Func, typename... Args>
    void fuzzTest(Func testFunc, int iterations,
                  std::function<Args()>... argGenerators);

    auto generateIPv4Address() -> std::string;

    auto generateMACAddress() -> std::string;

    auto generateURL() -> std::string;

    auto generateNormalDistribution(int count, double mean,
                                    double stddev) -> std::vector<double>;

    auto generateExponentialDistribution(int count,
                                         double lambda) -> std::vector<double>;

    template <typename T>
    auto serializeToJSON(const T& data) -> std::string;

    template <typename T>
    auto generateVector(int count,
                        std::function<T()> generator) -> std::vector<T>;

    template <typename K, typename V>
    auto generateMap(int count, std::function<K()> keyGenerator,
                     std::function<V()> valueGenerator) -> std::map<K, V>;

    template <typename T>
    auto generateSet(int count, std::function<T()> generator) -> std::set<T>;

    template <typename T, typename Distribution>
    auto generateCustomDistribution(int count, Distribution& distribution)
        -> std::vector<T>;

    template <typename T>
    auto generateSortedVector(int count,
                              std::function<T()> generator) -> std::vector<T>;

    template <typename T>
    auto generateUniqueVector(int count,
                              std::function<T()> generator) -> std::vector<T>;

    struct TreeNode {
        int value;
        std::vector<TreeNode> children = {};
    };

    auto generateTree(int depth, int maxChildren) -> TreeNode;

    auto generateGraph(int nodes,
                       double edgeProbability) -> std::vector<std::vector<int>>;

    auto generateKeyValuePairs(int count)
        -> std::vector<std::pair<std::string, std::string>>;

private:
    static void serializeToJSONHelper(std::ostringstream& oss,
                                      const std::string& str);

    static void serializeToJSONHelper(std::ostringstream& oss, int number);

    static void serializeToJSONHelper(std::ostringstream& oss, double number);

    static void serializeToJSONHelper(std::ostringstream& oss, bool boolean);

    template <typename T>
    static void serializeToJSONHelper(std::ostringstream& oss,
                                      const std::vector<T>& vec);

    template <typename K, typename V>
    static void serializeToJSONHelper(std::ostringstream& oss,
                                      const std::map<K, V>& map);
};

#endif  // ATOM_TESTS_FUZZ_HPP