#include "fuzz.hpp"

#include <ranges>

RandomDataGenerator::RandomDataGenerator(int seed)
    : generator_(seed),
      intDistribution_(0, DEFAULT_INT_MAX),
      realDistribution_(0.0, 1.0),
      charDistribution_(CHAR_MIN, CHAR_MAX) {}

auto RandomDataGenerator::generateIntegers(int count, int min,
                                           int max) -> std::vector<int> {
    std::uniform_int_distribution<> customDistribution(min, max);
    std::vector<int> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(customDistribution(generator_));
    }
    return result;
}

auto RandomDataGenerator::generateReals(int count, double min,
                                        double max) -> std::vector<double> {
    std::uniform_real_distribution<> customDistribution(min, max);
    std::vector<double> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(customDistribution(generator_));
    }
    return result;
}

auto RandomDataGenerator::generateString(int length,
                                         bool alphanumeric) -> std::string {
    std::string chars =
        alphanumeric
            ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwx"
              "yz"
            : []() {
                  std::string result(95, ' ');
                  for (char i = CHAR_MIN; i < CHAR_MIN + 95; ++i) {
                      result[i - CHAR_MIN] = i;
                  }
                  return result;
              }();
    std::uniform_int_distribution<> customDistribution(0, chars.size() - 1);
    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        result.push_back(chars[customDistribution(generator_)]);
    }
    return result;
}

auto RandomDataGenerator::generateBooleans(int count) -> std::vector<bool> {
    // return std::views::iota(0, count) | std::views::transform([this](auto) {
    //            return std::bernoulli_distribution(0.5)(generator_);
    //        }) |
    //        std::ranges::to<std::vector>();
    std::vector<bool> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(std::bernoulli_distribution(0.5)(generator_));
    }
    return result;
}

auto RandomDataGenerator::generateException() -> std::string {
    std::uniform_int_distribution<> exceptionType(0, 3);
    switch (exceptionType(generator_)) {
        case 0:
            throw std::runtime_error("Runtime Error");
        case 1:
            throw std::invalid_argument("Invalid Argument");
        case 2:
            throw std::out_of_range("Out of Range");
        default:
            throw std::exception();
    }
}

auto RandomDataGenerator::generateDateTime(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end)
    -> std::chrono::system_clock::time_point {
    auto duration =
        std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::uniform_int_distribution<long long> distribution(0, duration.count());
    return start + std::chrono::seconds(distribution(generator_));
}

auto RandomDataGenerator::generateRegexMatch(const std::string& regexStr)
    -> std::string {
    std::string result;
    for (char character : regexStr) {
        switch (character) {
            case '.':
                result += static_cast<char>(charDistribution_(generator_));
                break;
            case 'd':
                result += static_cast<char>(
                    std::uniform_int_distribution<>(48, 57)(generator_));
                break;
            case 'w':
                result += static_cast<char>(
                    std::uniform_int_distribution<>(97, 122)(generator_));
                break;
            default:
                result += character;
        }
    }
    return result;
}

auto RandomDataGenerator::generateFilePath(const std::string& baseDir,
                                           int depth) -> std::filesystem::path {
    std::filesystem::path path(baseDir);
    for (int i = 0; i < depth; ++i) {
        path /= generateString(FILE_PATH_SEGMENT_LENGTH, true);
    }
    path += "." + generateString(FILE_PATH_EXTENSION_LENGTH, true);
    return path;
}

auto RandomDataGenerator::generateRandomJSON(int depth) -> std::string {
    if (depth == 0) {
        return "\"value" + generateString(3, true) + "\"";
    }

    std::ostringstream oss;
    oss << "{";
    int elements = intDistribution_(generator_) % 4 + 1;
    for (int i = 0; i < elements; ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << "\"key" << generateString(3, true) << "\":";
        if (intDistribution_(generator_) % 2 == 0) {
            oss << generateRandomJSON(depth - 1);
        } else {
            oss << "\"" << generateString(5, true) << "\"";
        }
    }
    oss << "}";
    return oss.str();
}

auto RandomDataGenerator::generateRandomXML(int depth) -> std::string {
    if (depth == 0) {
        return "<element>" + generateString(5, true) + "</element>";
    }

    std::ostringstream oss;
    oss << "<element>";
    int elements = intDistribution_(generator_) % 3 + 1;
    for (int i = 0; i < elements; ++i) {
        if (intDistribution_(generator_) % 2 == 0) {
            oss << generateRandomXML(depth - 1);
        } else {
            oss << "<leaf>" << generateString(5, true) << "</leaf>";
        }
    }
    oss << "</element>";
    return oss.str();
}

auto RandomDataGenerator::generateIPv4Address() -> std::string {
    return std::to_string(intDistribution_(generator_) % IPV4_SEGMENT_MAX) +
           "." +
           std::to_string(intDistribution_(generator_) % IPV4_SEGMENT_MAX) +
           "." +
           std::to_string(intDistribution_(generator_) % IPV4_SEGMENT_MAX) +
           "." +
           std::to_string(intDistribution_(generator_) % IPV4_SEGMENT_MAX);
}

auto RandomDataGenerator::generateMACAddress() -> std::string {
    std::ostringstream oss;
    for (int i = 0; i < MAC_SEGMENTS; ++i) {
        if (i > 0) {
            oss << ":";
        }
        oss << std::hex << std::setw(2) << std::setfill('0')
            << intDistribution_(generator_) % MAC_SEGMENT_MAX;
    }
    return oss.str();
}

auto RandomDataGenerator::generateURL() -> std::string {
    static const std::vector<std::string> PROTOCOLS = {"http", "https"};
    static const std::vector<std::string> TLDS = {"com", "org", "net", "io"};

    std::string protocol =
        PROTOCOLS[intDistribution_(generator_) % PROTOCOLS.size()];
    std::string domain = generateString(URL_DOMAIN_LENGTH, true);
    std::string tld = TLDS[intDistribution_(generator_) % TLDS.size()];

    return protocol + "://www." + domain + "." + tld;
}

auto RandomDataGenerator::generateNormalDistribution(
    int count, double mean, double stddev) -> std::vector<double> {
    std::normal_distribution<> distribution(mean, stddev);
    return generateCustomDistribution<double>(count, distribution);
}

auto RandomDataGenerator::generateExponentialDistribution(
    int count, double lambda) -> std::vector<double> {
    std::exponential_distribution<> distribution(lambda);
    return generateCustomDistribution<double>(count, distribution);
}

void RandomDataGenerator::serializeToJSONHelper(std::ostringstream& oss,
                                                const std::string& str) {
    oss << '"' << str << '"';
}

void RandomDataGenerator::serializeToJSONHelper(std::ostringstream& oss,
                                                int number) {
    oss << number;
}

void RandomDataGenerator::serializeToJSONHelper(std::ostringstream& oss,
                                                double number) {
    oss << std::fixed << std::setprecision(JSON_PRECISION) << number;
}

void RandomDataGenerator::serializeToJSONHelper(std::ostringstream& oss,
                                                bool boolean) {
    oss << (boolean ? "true" : "false");
}

auto RandomDataGenerator::generateTree(int depth, int maxChildren) -> TreeNode {
    TreeNode root{intDistribution_(generator_)};
    if (depth > 0) {
        int numChildren = intDistribution_(generator_) % (maxChildren + 1);
        for (int i = 0; i < numChildren; ++i) {
            root.children.push_back(generateTree(depth - 1, maxChildren));
        }
    }
    return root;
}

auto RandomDataGenerator::generateGraph(int nodes, double edgeProbability)
    -> std::vector<std::vector<int>> {
    std::vector<std::vector<int>> adjacencyList(nodes);
    for (int i = 0; i < nodes; ++i) {
        for (int j = i + 1; j < nodes; ++j) {
            if (realDistribution_(generator_) < edgeProbability) {
                adjacencyList[i].push_back(j);
                adjacencyList[j].push_back(i);
            }
        }
    }
    return adjacencyList;
}

auto RandomDataGenerator::generateKeyValuePairs(int count)
    -> std::vector<std::pair<std::string, std::string>> {
    std::vector<std::pair<std::string, std::string>> pairs;
    for (int i = 0; i < count; ++i) {
        pairs.emplace_back(generateString(5, true), generateString(8, true));
    }
    return pairs;
}
