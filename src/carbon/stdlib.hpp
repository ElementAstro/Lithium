

#ifndef CARBON_STDLIB_HPP
#define CARBON_STDLIB_HPP

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "defines.hpp"
#include "language/common.hpp"

#include "command/function_call.hpp"

// #include "command/dispatchkit.hpp"
#include "command/bootstrap.hpp"
#include "command/bootstrap_stl.hpp"
#include "command/operators.hpp"
// #include "command/boxed_value.hpp"
#include "command/register_function.hpp"
#include "language/prelude.hpp"
#include "utils/json_wrap.hpp"

#ifndef CARBON_NO_THREADS
#include <future>
#endif\

/// @file
///
/// This file generates the standard library that normal ChaiScript usage
/// requires.

namespace Carbon {
class Std_Lib {
public:
    [[nodiscard]] static ModulePtr library() {
        auto lib = std::make_shared<Module>();
        bootstrap::Bootstrap::bootstrap(*lib);

        bootstrap::standard_library::vector_type<std::vector<Boxed_Value>>(
            "Vector", *lib);
        bootstrap::standard_library::string_type<std::string>("string", *lib);
        bootstrap::standard_library::map_type<
            std::map<std::string, Boxed_Value>>("Map", *lib);
        bootstrap::standard_library::map_type<
            std::unordered_map<Boxed_Value, Boxed_Value>>("HashMap", *lib);
        bootstrap::standard_library::pair_type<
            std::pair<Boxed_Value, Boxed_Value>>("Pair", *lib);

#ifndef CARBON_NO_THREADS
        bootstrap::standard_library::future_type<
            std::future<Carbon::Boxed_Value>>("future", *lib);
        lib->add(
            Carbon::fun(
                [](const std::function<Carbon::Boxed_Value()> &t_func) {
                    return std::async(std::launch::async, t_func);
                }),
            "async");
#endif

        json_wrap::library(*lib);

        lib->eval(Carbon_Prelude::prelude() /*, "standard prelude"*/);

        return lib;
    }
};
}  // namespace Carbon

#endif
