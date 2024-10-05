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

/**
 * @class RandomDataGenerator
 * @brief A class for generating random data for testing purposes.
 */
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
    /**
     * @brief Constructs a RandomDataGenerator with an optional seed.
     * @param seed The seed for the random number generator.
     */
    explicit RandomDataGenerator(int seed = std::random_device{}());

    /**
     * @brief Generates a vector of random integers.
     * @param count The number of integers to generate.
     * @param min The minimum value of the integers.
     * @param max The maximum value of the integers.
     * @return A vector of random integers.
     */
    auto generateIntegers(int count, int min = 0,
                          int max = DEFAULT_INT_MAX) -> std::vector<int>;

    /**
     * @brief Generates a vector of random real numbers.
     * @param count The number of real numbers to generate.
     * @param min The minimum value of the real numbers.
     * @param max The maximum value of the real numbers.
     * @return A vector of random real numbers.
     */
    auto generateReals(int count, double min = 0.0,
                       double max = 1.0) -> std::vector<double>;

    /**
     * @brief Generates a random string.
     * @param length The length of the string.
     * @param alphanumeric If true, the string will contain only alphanumeric
     * characters.
     * @return A random string.
     */
    auto generateString(int length, bool alphanumeric = false) -> std::string;

    /**
     * @brief Generates a vector of random boolean values.
     * @param count The number of boolean values to generate.
     * @return A vector of random boolean values.
     */
    auto generateBooleans(int count) -> std::vector<bool>;

    /**
     * @brief Generates a random exception message.
     * @return A random exception message.
     */
    auto generateException() -> std::string;

    /**
     * @brief Generates a random date and time within a specified range.
     * @param start The start of the date and time range.
     * @param end The end of the date and time range.
     * @return A random date and time within the specified range.
     */
    auto generateDateTime(const std::chrono::system_clock::time_point& start,
                          const std::chrono::system_clock::time_point& end)
        -> std::chrono::system_clock::time_point;

    /**
     * @brief Generates a string that matches a given regular expression.
     * @param regexStr The regular expression.
     * @return A string that matches the regular expression.
     */
    auto generateRegexMatch(const std::string& regexStr) -> std::string;

    /**
     * @brief Generates a random file path.
     * @param baseDir The base directory for the file path.
     * @param depth The depth of the file path.
     * @return A random file path.
     */
    auto generateFilePath(const std::string& baseDir,
                          int depth = 3) -> std::filesystem::path;

    /**
     * @brief Generates a random JSON string.
     * @param depth The depth of the JSON structure.
     * @return A random JSON string.
     */
    auto generateRandomJSON(int depth = 2) -> std::string;

    /**
     * @brief Generates a random XML string.
     * @param depth The depth of the XML structure.
     * @return A random XML string.
     */
    auto generateRandomXML(int depth = 2) -> std::string;

    /**
     * @brief Performs a fuzz test on a given function.
     * @tparam Func The type of the function to test.
     * @tparam Args The types of the arguments to the function.
     * @param testFunc The function to test.
     * @param iterations The number of iterations to run the test.
     * @param argGenerators The generators for the function arguments.
     */
    template <typename Func, typename... Args>
    void fuzzTest(Func testFunc, int iterations,
                  std::function<Args()>... argGenerators);

    /**
     * @brief Generates a random IPv4 address.
     * @return A random IPv4 address.
     */
    auto generateIPv4Address() -> std::string;

    /**
     * @brief Generates a random MAC address.
     * @return A random MAC address.
     */
    auto generateMACAddress() -> std::string;

    /**
     * @brief Generates a random URL.
     * @return A random URL.
     */
    auto generateURL() -> std::string;

    /**
     * @brief Generates a vector of random numbers following a normal
     * distribution.
     * @param count The number of numbers to generate.
     * @param mean The mean of the distribution.
     * @param stddev The standard deviation of the distribution.
     * @return A vector of random numbers following a normal distribution.
     */
    auto generateNormalDistribution(int count, double mean,
                                    double stddev) -> std::vector<double>;

    /**
     * @brief Generates a vector of random numbers following an exponential
     * distribution.
     * @param count The number of numbers to generate.
     * @param lambda The rate parameter of the distribution.
     * @return A vector of random numbers following an exponential distribution.
     */
    auto generateExponentialDistribution(int count,
                                         double lambda) -> std::vector<double>;

    /**
     * @brief Serializes data to a JSON string.
     * @tparam T The type of the data.
     * @param data The data to serialize.
     * @return A JSON string representing the data.
     */
    template <typename T>
    auto serializeToJSON(const T& data) -> std::string;

    /**
     * @brief Generates a vector of random elements.
     * @tparam T The type of the elements.
     * @param count The number of elements to generate.
     * @param generator The generator function for the elements.
     * @return A vector of random elements.
     */
    template <typename T>
    auto generateVector(int count,
                        std::function<T()> generator) -> std::vector<T>;

    /**
     * @brief Generates a map of random key-value pairs.
     * @tparam K The type of the keys.
     * @tparam V The type of the values.
     * @param count The number of key-value pairs to generate.
     * @param keyGenerator The generator function for the keys.
     * @param valueGenerator The generator function for the values.
     * @return A map of random key-value pairs.
     */
    template <typename K, typename V>
    auto generateMap(int count, std::function<K()> keyGenerator,
                     std::function<V()> valueGenerator) -> std::map<K, V>;

    /**
     * @brief Generates a set of random elements.
     * @tparam T The type of the elements.
     * @param count The number of elements to generate.
     * @param generator The generator function for the elements.
     * @return A set of random elements.
     */
    template <typename T>
    auto generateSet(int count, std::function<T()> generator) -> std::set<T>;

    /**
     * @brief Generates a vector of random elements following a custom
     * distribution.
     * @tparam T The type of the elements.
     * @tparam Distribution The type of the distribution.
     * @param count The number of elements to generate.
     * @param distribution The distribution to use.
     * @return A vector of random elements following the custom distribution.
     */
    template <typename T, typename Distribution>
    auto generateCustomDistribution(int count, Distribution& distribution)
        -> std::vector<T>;

    /**
     * @brief Generates a sorted vector of random elements.
     * @tparam T The type of the elements.
     * @param count The number of elements to generate.
     * @param generator The generator function for the elements.
     * @return A sorted vector of random elements.
     */
    template <typename T>
    auto generateSortedVector(int count,
                              std::function<T()> generator) -> std::vector<T>;

    /**
     * @brief Generates a vector of unique random elements.
     * @tparam T The type of the elements.
     * @param count The number of elements to generate.
     * @param generator The generator function for the elements.
     * @return A vector of unique random elements.
     */
    template <typename T>
    auto generateUniqueVector(int count,
                              std::function<T()> generator) -> std::vector<T>;

    /**
     * @struct TreeNode
     * @brief A structure representing a node in a tree.
     */
    struct TreeNode {
        int value;                            ///< The value of the node.
        std::vector<TreeNode> children = {};  ///< The children of the node.
    };

    /**
     * @brief Generates a random tree.
     * @param depth The depth of the tree.
     * @param maxChildren The maximum number of children per node.
     * @return A random tree.
     */
    auto generateTree(int depth, int maxChildren) -> TreeNode;

    /**
     * @brief Generates a random graph.
     * @param nodes The number of nodes in the graph.
     * @param edgeProbability The probability of an edge between any two nodes.
     * @return A random graph represented as an adjacency matrix.
     */
    auto generateGraph(int nodes,
                       double edgeProbability) -> std::vector<std::vector<int>>;

    /**
     * @brief Generates a vector of random key-value pairs.
     * @return A vector of random key-value pairs.
     */
    auto generateKeyValuePairs(int count)
        -> std::vector<std::pair<std::string, std::string>>;

private:
    /**
     * @brief Helper function to serialize a string to JSON.
     * @param oss The output string stream.
     * @param str The string to serialize.
     */
    static void serializeToJSONHelper(std::ostringstream& oss,
                                      const std::string& str);

    /**
     * @brief Helper function to serialize an integer to JSON.
     * @param oss The output string stream.
     * @param number The integer to serialize.
     */
    static void serializeToJSONHelper(std::ostringstream& oss, int number);

    /**
     * @brief Helper function to serialize a double to JSON.
     * @param oss The output string stream.
     * @param number The double to serialize.
     */
    static void serializeToJSONHelper(std::ostringstream& oss, double number);

    /**
     * @brief Helper function to serialize a boolean to JSON.
     * @param oss The output string stream.
     * @param boolean The boolean to serialize.
     */
    static void serializeToJSONHelper(std::ostringstream& oss, bool boolean);

    /**
     * @brief Helper function to serialize a vector to JSON.
     * @tparam T The type of the elements in the vector.
     * @param oss The output string stream.
     * @param vec The vector to serialize.
     */
    template <typename T>
    static void serializeToJSONHelper(std::ostringstream& oss,
                                      const std::vector<T>& vec);

    /**
     * @brief Helper function to serialize a map to JSON.
     * @tparam K The type of the keys in the map.
     * @tparam V The type of the values in the map.
     * @param oss The output string stream.
     * @param map The map to serialize.
     */
    template <typename K, typename V>
    static void serializeToJSONHelper(std::ostringstream& oss,
                                      const std::map<K, V>& map);
};

#endif  // ATOM_TESTS_FUZZ_HPP
