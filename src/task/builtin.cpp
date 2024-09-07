#include "builtin.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

#include "atom/type/json.hpp"

namespace lithium {

BuiltinFunctions::BuiltinFunctions() {
    registerMathFunctions();
    registerStringFunctions();
    registerArrayFunctions();
}

auto BuiltinFunctions::executeFunction(const std::string& name, const nlohmann::json& args) -> nlohmann::json {
    if (functions_.find(name) == functions_.end()) {
        LITHIUM_THROW(std::runtime_error, "Unknown builtin function: {}", name);
    }
    return functions_[name](args);
}

void BuiltinFunctions::registerMathFunctions() {
    functions_["math_sin"] = [](const nlohmann::json& args) -> json {
        return std::sin(args[0].get<double>());
    };
    functions_["math_cos"] = [](const nlohmann::json& args) {
        return std::cos(args[0].get<double>());
    };
    functions_["math_tan"] = [](const nlohmann::json& args) {
        return std::tan(args[0].get<double>());
    };
    functions_["math_pow"] = [](const nlohmann::json& args) {
        return std::pow(args[0].get<double>(), args[1].get<double>());
    };
    // Add more math functions as needed
}

void BuiltinFunctions::registerStringFunctions() {
    functions_["string_length"] = [](const nlohmann::json& args) {
        return args[0].get<std::string>().length();
    };
    functions_["string_to_upper"] = [](const nlohmann::json& args) {
        std::string result = args[0].get<std::string>();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    };
    functions_["string_to_lower"] = [](const nlohmann::json& args) {
        std::string result = args[0].get<std::string>();
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    };
    // Add more string functions as needed
}

} // namespace lithium
