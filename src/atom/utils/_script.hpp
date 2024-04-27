/*
 * _script.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: Carbon binding for Atom-Utils

**************************************************/

#ifndef ATOM_TYPE_SCRIPT_HPP
#define ATOM_TYPE_SCRIPT_HPP

#include "carbon/carbon.hpp"

#include "aes.hpp"
#include "argsview.hpp"
#include "env.hpp"
#include "random.hpp"
#include "stopwatcher.hpp"
#include "string.hpp"
#include "time.hpp"
#include "utf.hpp"
#include "uuid.hpp"
#include "xml.hpp"

using namespace Atom::Utils;

namespace Atom::_Script::Utils {
/**
 * Adds the String Methods to the given Carbon module.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {
    m->add(Carbon::fun(&encryptAES), "encrypt_aes");
    m->add(Carbon::fun(&decryptAES), "decrypt_aes");
    m->add(Carbon::fun(&compress), "compress");
    m->add(Carbon::fun(&decompress), "decompress");
    m->add(Carbon::fun(&calculateSha256), "calculate_sha256");

    m->add(user_type<ArgsView>(), "ArgsView");
    m->add(Carbon::fun<std::optional<std::string_view> (ArgsView::*)(
               std::string_view) const>(&ArgsView::get),
           "get_string");
    m->add(Carbon::fun(&ArgsView::get<int>), "get_int");
    m->add(Carbon::fun(&ArgsView::get<double>), "get_double");
    m->add(Carbon::fun(&ArgsView::get<bool>), "get_bool");
    m->add(Carbon::fun(&ArgsView::has), "has");
    m->add(Carbon::fun(&ArgsView::hasFlag), "has_flag");
    m->add(Carbon::fun(&ArgsView::addRule), "add_rule");
    m->add(Carbon::fun(&ArgsView::getFlags), "get_flags");

    m->add(user_type<Env>(), "Env");
    m->add(Carbon::fun(&Env::setEnv), "set_env");
    m->add(Carbon::fun(&Env::getEnv), "get_env");
    m->add(Carbon::fun(&Env::add), "add");
    m->add(Carbon::fun(&Env::del), "del");
    m->add(Carbon::fun(&Env::get), "get");
    m->add(Carbon::fun(&Env::getAbsolutePath), "get_absolute_path");
    m->add(Carbon::fun(&Env::getAbsoluteWorkPath), "get_absolute_work_path");
    m->add(Carbon::fun(&Env::getConfigPath), "get_config_path");
    m->add(Carbon::fun(&Env::removeHelp), "remove_help");
    m->add(Carbon::fun(&Env::addHelp), "add_help");
    m->add(Carbon::fun(&Env::printHelp), "print_help");
    m->add(Carbon::fun(&Env::createShared), "create_shared");
    m->add(Carbon::fun(&Env::createUnique), "create_unique");

    m->add(user_type<std::default_random_engine>(), "default_random_engine");
    m->add(user_type<std::uniform_int_distribution<>>(),
           "uniform_int_distribution");
    m->add(user_type<random<>>(), "random");
    m->add(Carbon::fun(
               static_cast<
                   typename random<std::default_random_engine,
                                   std::uniform_int_distribution<>>::
                       result_type (random<std::default_random_engine,
                                           std::uniform_int_distribution<>>::*)(
                           const typename random<
                               std::default_random_engine,
                               std::uniform_int_distribution<>>::param_type &)>(
                   &random<std::default_random_engine,
                           std::uniform_int_distribution<>>::operator())),
           "operator()");
    m->add(Carbon::fun(&generateRandomString), "generate_random_string");

    m->add(user_type<StopWatcher>(), "StopWatcher");
    m->add(Carbon::fun(&StopWatcher::start), "start");
    m->add(Carbon::fun(&StopWatcher::pause), "pause");
    m->add(Carbon::fun(&StopWatcher::stop), "stop");
    m->add(Carbon::fun(&StopWatcher::resume), "resume");
    m->add(Carbon::fun(&StopWatcher::reset), "reset");
    m->add(Carbon::fun(&StopWatcher::elapsedMilliseconds), "elapsed_ms");
    m->add(Carbon::fun(&StopWatcher::elapsedSeconds), "elapsed_s");
    m->add(Carbon::fun(&StopWatcher::elapsedFormatted), "elapsed_formatted");
    m->add(Carbon::fun(&StopWatcher::registerCallback), "register_callback");

    m->add(Carbon::fun(&hasUppercase), "has_uppercase");
    m->add(Carbon::fun(&toCamelCase), "to_camel_case");
    m->add(Carbon::fun(&toUnderscore), "to_underscore");

    m->add(Carbon::fun(&urlEncode), "url_encode");
    m->add(Carbon::fun(&urlDecode), "url_decode");
    m->add(Carbon::fun(&replaceString), "replace_string");
    m->add(Carbon::fun(&replaceStrings), "replace_strings");
    m->add(Carbon::fun(&startsWith), "starts_with");
    m->add(Carbon::fun(&endsWith), "ends_with");
    m->add(Carbon::fun(&joinStrings), "join_strings");
    m->add(Carbon::fun(&splitString), "split_string");

    m->add(user_type<std::tm>(), "tm");
    m->add_global_const(Carbon::const_var(&std::tm::tm_sec), "tm_sec");
    m->add_global_const(Carbon::const_var(&std::tm::tm_min), "tm_min");
    m->add_global_const(Carbon::const_var(&std::tm::tm_hour), "tm_hour");
    m->add_global_const(Carbon::const_var(&std::tm::tm_mday), "tm_mday");
    m->add_global_const(Carbon::const_var(&std::tm::tm_mon), "tm_mon");
    m->add_global_const(Carbon::const_var(&std::tm::tm_year), "tm_year");
    m->add_global_const(Carbon::const_var(&std::tm::tm_wday), "tm_wday");
    m->add_global_const(Carbon::const_var(&std::tm::tm_yday), "tm_yday");
    m->add_global_const(Carbon::const_var(&std::tm::tm_isdst), "tm_isdst");
    m->add(Carbon::fun(&timeStampToString), "time_stamp_to_string");
    m->add(Carbon::fun(&getTimestampString), "get_timestamp_string");
    m->add(Carbon::fun(&getChinaTimestampString), "get_china_timestamp_string");
    m->add(Carbon::fun(&getUtcTime), "get_utc_time");
    m->add(Carbon::fun(&timestampToTime), "timestamp_to_time");
    m->add(Carbon::fun(&toString), "to_string");
    m->add(Carbon::fun(&convertToChinaTime), "convert_to_china_time");

    m->add(Carbon::fun(&toUTF8), "to_utf8");
    m->add(Carbon::fun(&fromUTF8), "from_utf8");
    m->add(Carbon::fun(&UTF8toUTF16), "utf8_to_utf16");
    m->add(Carbon::fun(&UTF16toUTF8), "utf16_to_utf8");
    m->add(Carbon::fun(&UTF8toUTF32), "utf8_to_utf32");
    m->add(Carbon::fun(&UTF32toUTF8), "utf32_to_utf8");
    m->add(Carbon::fun(&UTF16toUTF32), "utf16_to_utf32");
    m->add(Carbon::fun(&UTF32toUTF16), "utf32_to_utf16");

    m->add(user_type<UUIDGenerator>(), "UUIDGenerator");
    m->add(Carbon::fun(&UUIDGenerator::generateUUID), "generate_uuid");
    m->add(Carbon::fun(&UUIDGenerator::getRandomNumber), "get_random_number");
    m->add(Carbon::fun(&UUIDGenerator::seed), "seed");
    m->add(Carbon::fun(&generateSystemUUID), "generate_system_uuid");

    m->add(user_type<XMLReader>(), "XMLReader");
    m->add(Carbon::fun(&XMLReader::getChildElementText),
           "get_child_element_text");
    m->add(Carbon::fun(&XMLReader::getChildElementAttributeValue),
           "get_child_element_attribute_value");
    m->add(Carbon::fun(&XMLReader::getChildElementAttributeValueByPath),
           "get_child_element_attribute_value_by_path");
    m->add(Carbon::fun(&XMLReader::getChildElementTextByPath),
           "get_child_element_text_by_path");
    m->add(Carbon::fun(&XMLReader::getChildElementNames),
           "get_child_element_names");
    m->add(Carbon::fun(&XMLReader::getRootElementNames),
           "get_root_element_names");
    m->add(Carbon::fun(&XMLReader::getAttributeValue), "get_attribute_value");
    m->add(Carbon::fun(&XMLReader::getAttributeValueByPath),
           "get_attribute_value_by_path");
    m->add(Carbon::fun(&XMLReader::getValueByPath), "get_value_by_path");
    m->add(Carbon::fun(&XMLReader::hasChildElement), "has_child_element");
    m->add(Carbon::fun(&XMLReader::hasChildElementByPath),
           "has_child_element_by_path");
    m->add(Carbon::fun(&XMLReader::saveToFile), "save_to_file");
    m->add(Carbon::fun(&XMLReader::getElementText), "get_element_text");

    return m;
}
}  // namespace Atom::_Script::Utils

#endif