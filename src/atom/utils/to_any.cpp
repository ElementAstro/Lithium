#include "to_any.hpp"

#include <iomanip>
#include <map>
#include <set>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace atom::utils {
// Impl Class Implementation
class Parser::Impl {
public:
    auto parseLiteral(const std::string& input) -> std::optional<std::any>;
    auto parseLiteralWithDefault(const std::string& input,
                                 const std::any& defaultValue) -> std::any;
    void print(const std::any& value) const;
    void logParsing(const std::string& input, const std::any& result) const;
    auto convertToAnyVector(const std::vector<std::string>& input)
        -> std::vector<std::any>;
    void registerCustomParser(const std::string& type, CustomParserFunc parser);
    void printCustomParsers() const;

    // 新增解析函数
    void parseJson(const std::string& jsonString) const;
    void parseCsv(const std::string& csvString, char delimiter = ',') const;

private:
    std::unordered_map<std::string, CustomParserFunc> customParsers_;

    static auto trim(std::string_view str) -> std::string_view;
    auto fromString(std::string_view str) -> std::optional<std::any>;
    auto parseVector(std::string_view str) -> std::optional<std::vector<int>>;
    template <typename T>
    auto parseSingleValue(std::string_view str) -> std::optional<T>;
    template <typename T>
    auto parseVectorOf(std::string_view str) -> std::optional<std::vector<T>>;
    template <typename T>
    auto parseSetOf(std::string_view str) -> std::optional<std::set<T>>;
    template <typename K, typename V>
    auto parseMapOf(std::string_view str) -> std::optional<std::map<K, V>>;
    static auto split(const std::string& str,
                      char delimiter) -> std::vector<std::string>;
    static auto parseDateTime(std::string_view str)
        -> std::optional<std::chrono::system_clock::time_point>;
};

Parser::Parser() : pImpl_(std::make_unique<Impl>()) {}
Parser::~Parser() = default;

auto Parser::parseLiteral(const std::string& input) -> std::optional<std::any> {
    if (isProcessing_.exchange(true)) {
        THROW_PAESER_ERROR("Parser is currently processing another input.");
    }
    auto result = pImpl_->parseLiteral(input);
    isProcessing_ = false;
    return result;
}

auto Parser::parseLiteralWithDefault(const std::string& input,
                                     const std::any& defaultValue) -> std::any {
    auto result = parseLiteral(input);
    return result ? *result : defaultValue;
}

void Parser::print(const std::any& value) const { pImpl_->print(value); }

void Parser::logParsing(const std::string& input,
                        const std::any& result) const {
    pImpl_->logParsing(input, result);
}

auto Parser::convertToAnyVector(const std::vector<std::string>& input)
    -> std::vector<std::any> {
    return pImpl_->convertToAnyVector(input);
}

void Parser::registerCustomParser(const std::string& type,
                                  CustomParserFunc parser) {
    pImpl_->registerCustomParser(type, std::move(parser));
}

void Parser::parseJson(const std::string& jsonString) const {
    pImpl_->parseJson(jsonString);
}

void Parser::parseCsv(const std::string& csvString, char delimiter) const {
    pImpl_->parseCsv(csvString, delimiter);
}

void Parser::printCustomParsers() const { pImpl_->printCustomParsers(); }

// 实现解析函数方法
auto Parser::Impl::trim(std::string_view str) -> std::string_view {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string_view::npos)
               ? ""
               : str.substr(first, last - first + 1);
}

template <typename T>
auto Parser::Impl::parseSingleValue(std::string_view str) -> std::optional<T> {
    // 对算术类型使用 std::from_chars 解析
    if constexpr (std::is_arithmetic_v<T>) {
        T value;
        auto [ptr, ec] =
            std::from_chars(str.data(), str.data() + str.size(), value);

        // 检查解析是否成功
        if (ec == std::errc{}) {
            return value;
        }
        return std::nullopt;  // 解析失败，返回空值
    }
    // 如果是 std::string 类型，直接返回
    else if constexpr (std::is_same_v<T, std::string>) {
        return std::string(str);  // 将 std::string_view 转换为 std::string
    }

    // 如果类型不受支持，返回空值
    return std::nullopt;
}

auto Parser::Impl::fromString(std::string_view str) -> std::optional<std::any> {
    auto trimmed = trim(str);

    // 尝试解析各种类型
    if (auto intValue = parseSingleValue<int>(trimmed)) {
        return *intValue;
    }
    if (auto longValue = parseSingleValue<long>(trimmed)) {
        return *longValue;
    }
    if (auto longLongValue = parseSingleValue<long long>(trimmed)) {
        return *longLongValue;
    }
    if (auto unsignedValue = parseSingleValue<unsigned int>(trimmed)) {
        return *unsignedValue;
    }
    if (auto floatValue = parseSingleValue<float>(trimmed)) {
        return *floatValue;
    }
    if (auto doubleValue = parseSingleValue<double>(trimmed)) {
        return *doubleValue;
    }

    // 布尔值解析
    if (trimmed == "true") {
        return true;
    }
    if (trimmed == "false") {
        return false;
    }

    // 字符解析
    if (trimmed.size() == 1 && !std::isspace(trimmed[0])) {
        return trimmed.front();
    }

    // 解析日期时间
    if (auto dateTimeValue = parseDateTime(trimmed)) {
        return *dateTimeValue;
    }

    return std::string(trimmed);  // 默认为字符串
}

auto Parser::Impl::parseVector(std::string_view str)
    -> std::optional<std::vector<int>> {
    return parseVectorOf<int>(str);
}

template <typename T>
auto Parser::Impl::parseVectorOf(std::string_view str)
    -> std::optional<std::vector<T>> {
    std::vector<T> result;
    auto tokens = split(std::string(str), ',');
    for (const auto& token : tokens) {
        auto optValue = parseSingleValue<T>(trim(token));
        if (optValue) {
            result.push_back(*optValue);
        } else {
            return std::nullopt;  // 解析失败，返回空
        }
    }
    return result;
}

template <typename T>
auto Parser::Impl::parseSetOf(std::string_view str)
    -> std::optional<std::set<T>> {
    std::set<T> result;
    auto tokens = split(std::string(str), ',');
    for (const auto& token : tokens) {
        auto optValue = parseSingleValue<T>(trim(token));
        if (optValue) {
            result.insert(*optValue);
        } else {
            return std::nullopt;  // 解析失败，返回空
        }
    }
    return result;
}

template <typename K, typename V>
auto Parser::Impl::parseMapOf(std::string_view str)
    -> std::optional<std::map<K, V>> {
    std::map<K, V> result;
    auto pairs = split(std::string(str), ',');
    for (const auto& pair : pairs) {
        auto keyValue = split(pair, ':');
        if (keyValue.size() == 2) {
            auto key = trim(keyValue[0]);
            auto valueOpt = parseSingleValue<V>(trim(keyValue[1]));
            if (auto parsedKey = parseSingleValue<K>(key); valueOpt) {
                result[*parsedKey] = *valueOpt;
            } else {
                return std::nullopt;  // 解析失败，返回空
            }
        }
    }
    return result;
}

auto Parser::Impl::split(const std::string& str,
                         char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

auto Parser::Impl::parseLiteral(const std::string& input)
    -> std::optional<std::any> {
    // 优先检查自定义解析器
    for (const auto& [type, parserFunc] : customParsers_) {
        if (input.find(type) != std::string::npos) {
            LOG_F(INFO, "Using custom parser for type: {}", type);
            auto customValue = parserFunc(input);
            if (customValue) {
                LOG_F(INFO, "Custom parser succeeded");
                LOG_F(INFO, "Parsed input: '{}' as type: {}", input, type);
                LOG_F(INFO, "{}", customValue->type().name());
                return customValue.value();
            }
        }
    }

    if (auto result = fromString(input)) {
        return result;
    }

    // 尝试解析集合
    if (auto vectorResult = parseVector(input)) {
        return *vectorResult;
    }  // 对于 vector<int>
    if (auto setResult = parseSetOf<float>(input)) {
        return *setResult;
    }  // 示例：set<float>
    if (auto mapResult = parseMapOf<std::string, int>(input)) {
        return *mapResult;
    }  // 示例：map<string, int>

    return std::nullopt;  // 如果没有匹配
}

void Parser::Impl::print(const std::any& value) const {
    LOG_F(INFO, "Parsed value: ");
    LOG_F(INFO, "{} - ", value.type().name());
    if (value.type() == typeid(int)) {
        LOG_F(INFO, "%d", std::any_cast<int>(value));
    } else if (value.type() == typeid(long)) {
        LOG_F(INFO, "%ld", std::any_cast<long>(value));
    } else if (value.type() == typeid(long long)) {
        LOG_F(INFO, "%lld", std::any_cast<long long>(value));
    } else if (value.type() == typeid(unsigned int)) {
        LOG_F(INFO, "%u", std::any_cast<unsigned int>(value));
    } else if (value.type() == typeid(float)) {
        LOG_F(INFO, "%f", std::any_cast<float>(value));
    } else if (value.type() == typeid(double)) {
        LOG_F(INFO, "%lf", std::any_cast<double>(value));
    } else if (value.type() == typeid(bool)) {
        LOG_F(INFO, "{}", std::any_cast<bool>(value) ? "true" : "false");
    } else if (value.type() == typeid(char)) {
        LOG_F(INFO, "'%c'", std::any_cast<char>(value));
    } else if (value.type() == typeid(std::string)) {
        LOG_F(INFO, "{}", std::any_cast<std::string>(value));
    } else if (value.type() == typeid(std::optional<std::string>)) {
        LOG_F(INFO, "{}",
              std::any_cast<std::optional<std::string>>(value).value());
    } else if (value.type() == typeid(std::chrono::system_clock::time_point)) {
        auto time = std::any_cast<std::chrono::system_clock::time_point>(value);
        std::time_t timeT = std::chrono::system_clock::to_time_t(time);
        LOG_F(INFO, "{}", std::ctime(&timeT));  // 转换为字符串并输出
    } else {
        LOG_F(INFO, "Unknown type");
    }
}

void Parser::Impl::logParsing(const std::string& input,
                              const std::any& result) const {
    LOG_F(INFO, "Parsed input: '{}' as type: ", input);

    if (result.type() == typeid(int)) {
        LOG_F(INFO, "int");
    } else if (result.type() == typeid(long)) {
        LOG_F(INFO, "long");
    } else if (result.type() == typeid(long long)) {
        LOG_F(INFO, "long long");
    } else if (result.type() == typeid(unsigned int)) {
        LOG_F(INFO, "unsigned int");
    } else if (result.type() == typeid(float)) {
        LOG_F(INFO, "float");
    } else if (result.type() == typeid(double)) {
        LOG_F(INFO, "double");
    } else if (result.type() == typeid(bool)) {
        LOG_F(INFO, "bool");
    } else if (result.type() == typeid(char)) {
        LOG_F(INFO, "char");
    } else if (result.type() == typeid(std::string)) {
        LOG_F(INFO, "string");
    } else if (result.type() == typeid(std::chrono::system_clock::time_point)) {
        auto time =
            std::any_cast<std::chrono::system_clock::time_point>(result);
        std::time_t timeT = std::chrono::system_clock::to_time_t(time);
        LOG_F(INFO, "{}", std::ctime(&timeT));  // 转换为字符串并输出
    } else if (result.type() == typeid(std::vector<int>)) {
        LOG_F(INFO, "vector<int>");
    } else {
        LOG_F(INFO, "unknown");
    }
}

auto Parser::Impl::parseDateTime(std::string_view str)
    -> std::optional<std::chrono::system_clock::time_point> {
    std::tm timeStruct{};
    std::istringstream ss{std::string(
        str)};  // 使用 std::string_view 时需要显式转换为 std::string

    // 假设日期格式为 YYYY-MM-DD HH:MM:SS
    ss >> std::get_time(&timeStruct, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        return std::nullopt;  // 解析失败，返回空值
    }

    // 转换为 time_point
    timeStruct.tm_isdst = -1;  // 让 mktime 自行判断是否为夏令时
    std::time_t timeT = std::mktime(&timeStruct);

    if (timeT == -1) {
        return std::nullopt;  // mktime 转换失败
    }

    return std::chrono::system_clock::from_time_t(timeT);
}

void Parser::Impl::parseJson(const std::string& jsonString) const {
    try {
        auto json = json::parse(jsonString);
        LOG_F(INFO, "Parsed JSON: {}", json.dump());
        // 进一步处理 JSON 对象
    } catch (const json::parse_error& e) {
        THROW_PAESER_ERROR("Failed to parse JSON: " + std::string(e.what()));
    }
}

void Parser::Impl::parseCsv(const std::string& csvString,
                            char delimiter) const {
    std::istringstream stream(csvString);
    std::string line;

    while (std::getline(stream, line)) {
        auto values = split(line, delimiter);
        for (const auto& value : values) {
            LOG_F(INFO, "Parsed value: {}", value);
            // 此处可添加进一步处理逻辑
        }
    }
}

void Parser::Impl::registerCustomParser(const std::string& type,
                                        CustomParserFunc parser) {
    customParsers_[type] = parser;
}

auto Parser::Impl::convertToAnyVector(const std::vector<std::string>& input)
    -> std::vector<std::any> {
    std::vector<std::any> result;
    for (const auto& str : input) {
        auto parsedValue = parseLiteral(str);
        result.emplace_back(parsedValue ? *parsedValue
                                        : std::string("Invalid input: " + str));
    }
}

void Parser::Impl::printCustomParsers() const {
    for (const auto& [type, parserFunc] : customParsers_) {
        LOG_F(INFO, "Custom parser for type: {}", type);
    }
}

}  // namespace atom::utils
