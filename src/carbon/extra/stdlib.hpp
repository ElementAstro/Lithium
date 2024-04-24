#ifndef CARBON_EXTRAS_STDLIB_HPP
#define CARBON_EXTRAS_STDLIB_HPP

#include "../carbon.hpp"

#include <chrono>
#include <optional>
#include <string>

namespace Carbon::extras::stdlib {
template <typename T>
ModulePtr optional(ModulePtr m = std::make_shared<Module>(),
                   const std::string &type = "") {
    utility::add_class<std::optional<T>>(
        *m, std::string("Optional") + type,
        {constructor<std::optional<T>()>()

        },
        {
            {fun(&std::optional<T>::has_value), "has_value"},
            //{fun(&std::optional<T>::value), "value"},
            //{fun(&std::optional<T>::value_or), "value_or"},
            {fun(&std::optional<T>::swap), "swap"},
            {fun(&std::optional<T>::reset), "reset"},
    //{fun(&std::optional<T>::emplace), "emplace"},
    //{fun(&std::optional<T>::operator=), "operator="},
#if __cplusplus >= 202110L
            {fun(&std::optional<T>::and_then), "and_then"},
            {fun(&std::optional<T>::or_else), "or_else"},
            {fun(&std::optional<T>::transform), "transform"},
#endif
            //{fun(&std::make_optional<T>), std::string("make_Optional") + type}
        });
    return m;
}

ModulePtr timezone(ModulePtr m = std::make_shared<Module>()) {
    utility::add_class<std::chrono::tzdb>(
        *m, "Tzdb", {},
        {{fun(&std::chrono::tzdb::version), "version"},
         {fun(&std::chrono::tzdb::current_zone), "current_zone"},
         {fun(&std::chrono::tzdb::locate_zone), "locate_zone"},
         {fun(&std::chrono::tzdb::zones), "zones"},
         {fun(&std::chrono::tzdb::links), "links"},
         {fun(&std::chrono::tzdb::leap_seconds), "leap_seconds"}});

    utility::add_class<std::chrono::tzdb_list>(*m, "TimezoneList", {}, {});

    utility::add_class<std::chrono::sys_info>(
        *m, "SysInfo",
        {

        },
        {{fun(&std::chrono::sys_info::abbrev), "abbrev"},
         {fun(&std::chrono::sys_info::offset), "offset"},
         {fun(&std::chrono::sys_info::save), "save"}});

    utility::add_class<std::chrono::time_zone>(
        *m, "Timezone", {},
        {
            //{fun<std::chrono::sys_info (std::chrono::time_zone::*)(const
            //std::chrono::sys_time<std::chrono::seconds>&)
            //const>(&std::chrono::time_zone::get_info), "get_info"}
        });
    m->add(fun(&std::chrono::get_tzdb), "get_tzdb");
    m->add(fun(&std::chrono::get_tzdb_list), "get_tzdb_list");
    m->add(fun(&std::chrono::reload_tzdb), "reload_tzdb");
    m->add(fun(&std::chrono::remote_version), "remote_version");
    m->add(fun(&std::chrono::locate_zone), "locate_zone");
    m->add(fun(&std::chrono::current_zone), "current_zone");

    return m;
}
/**
 * Adds the String Methods to the given Carbon module.
 */
ModulePtr bootstrap(ModulePtr m = std::make_shared<Module>()) {
    optional<int>(m, "Int");
    optional<double>(m, "Double");
    optional<std::string>(m, "String");
    optional<bool>(m, "Bool");
    timezone(m);
    return m;
}
}  // namespace Carbon::extras::stdlib

#endif /* CARBON_EXTRAS_STDLIB_HPP */