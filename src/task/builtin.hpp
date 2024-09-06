#ifndef LITHIUM_TASK_INTERPRETER_BUILTINS_HPP
#define LITHIUM_TASK_INTERPRETER_BUILTINS_HPP

#include <functional>
#include <unordered_map>
#include "atom/type/json_fwd.hpp"

using json = nlohmann::json;

namespace lithium {

class BuiltinFunctions {
public:
    BuiltinFunctions();

    auto executeFunction(const std::string& name, const json& args) -> json;

private:
    std::unordered_map<std::string, std::function<json(const json&)>>
        functions_;

    void registerMathFunctions();
    void registerStringFunctions();
    void registerArrayFunctions();
};

}  // namespace lithium

#endif  // LITHIUM_TASK_INTERPRETER_BUILTINS_HPP