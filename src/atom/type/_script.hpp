/*
 * _script.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: Carbon binding for Atom-Type

**************************************************/

#ifndef ATOM_TYPE_SCRIPT_HPP
#define ATOM_TYPE_SCRIPT_HPP

#include "carbon/carbon.hpp"

#include "args.hpp"
#include "flatset.hpp"
#include "ini.hpp"
#include "small_vector.hpp"

using namespace atom::type;

namespace Atom::_Script::Type {
/**
 * Adds the String Methods to the given Carbon module.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {
    m->add(user_type<Args>(), "Args");
    m->add(Carbon::fun(&Args::set<Carbon::Boxed_Value>), "set");
    m->add(Carbon::fun(&Args::get<Carbon::Boxed_Value>), "get");
    m->add(Carbon::fun(&Args::get_or<Carbon::Boxed_Value>), "get_or");
    m->add(Carbon::fun(&Args::contains), "contains");
    m->add(Carbon::fun(&Args::get_optional<Carbon::Boxed_Value>),
           "get_optional");
    m->add(Carbon::fun(&Args::remove), "remove");
    m->add(Carbon::fun(&Args::empty), "empty");
    m->add(Carbon::fun(&Args::size), "size");
    m->add(Carbon::fun(&Args::clear), "clear");
    m->add(Carbon::fun(&Args::data), "data");

    //Carbon::bootstrap::standard_library::vector_type<
    //    SmallVector<Carbon::Boxed_Value>>("SmallVector", *m);
    //Carbon::bootstrap::standard_library::set_type<FlatSet<Carbon::Boxed_Value>>(
    //    "FlatSet", *m);
    //Carbon::bootstrap::standard_library::set_type<
    //    std::set<Carbon::Boxed_Value>>("Set", *m);

    m->add(user_type<INIFile>(), "INIFile");
    // m->add(Carbon::constructor<INIFile()>(), "INIFile");
    m->add(Carbon::fun(&INIFile::load), "load");
    m->add(Carbon::fun(&INIFile::save), "save");
    m->add(Carbon::fun(&INIFile::set<int>), "set_int");
    m->add(Carbon::fun(&INIFile::get<int>), "get_int");
    m->add(Carbon::fun(&INIFile::set<double>), "set_double");
    m->add(Carbon::fun(&INIFile::get<double>), "get_double");
    m->add(Carbon::fun(&INIFile::set<std::string>), "set_string");
    m->add(Carbon::fun(&INIFile::get<std::string>), "get_string");
    m->add(Carbon::fun(&INIFile::set<bool>), "get_bool");
    m->add(Carbon::fun(&INIFile::get<bool>), "get_bool");
    m->add(Carbon::fun(&INIFile::has), "has");
    m->add(Carbon::fun(&INIFile::hasSection), "has_section");
    m->add(Carbon::fun(&INIFile::toJson), "to_json");
    m->add(Carbon::fun(&INIFile::toXml), "to_xml");

    return m;
}
}  // namespace Atom::_Script::Type

#endif
