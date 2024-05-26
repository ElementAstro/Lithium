/*
 * _script.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: Carbon binding for Atom-ERROR

**************************************************/

#ifndef ATOM_ERROR_SCRIPT_HPP
#define ATOM_ERROR_SCRIPT_HPP

#include "carbon/carbon.hpp"

#include "error_code.hpp"
#include "error_stack.hpp"
#include "exception.hpp"

using namespace atom::error;

namespace Atom::_Script::Error {
/**
 * Adds the String Methods to the given Carbon module.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {
    m->add(
        Carbon::type_conversion<LIError, std::string>([](const LIError &error) {
            switch (error) {
                case LIError::None:
                    return "None";
                case LIError::NotFound:
                    return "NotFound";
                case LIError::OpenError:
                    return "OpenError";
                case LIError::AccessDenied:
                    return "AccessDenied";
                case LIError::ReadError:
                    return "ReadError";
                case LIError::WriteError:
                    return "WriteError";
                case LIError::PermissionDenied:
                    return "PermissionDenied";
                case LIError::ParseError:
                    return "ParseError";
                case LIError::InvalidPath:
                    return "InvalidPath";
                case LIError::FileExists:
                    return "FileExists";
                case LIError::DirectoryNotEmpty:
                    return "DirectoryNotEmpty";
                case LIError::TooManyOpenFiles:
                    return "TooManyOpenFiles";
                case LIError::DiskFull:
                    return "DiskFull";
                case LIError::LoadError:
                    return "LoadError";
                case LIError::UnLoadError:
                    return "UnLoadError";
            }
            return "Unknown";
        }));
    m->add(Carbon::type_conversion<std::string, LIError>(
        [](const std::string &str) {
            if (str == "None")
                return LIError::None;
            else if (str == "NotFound")
                return LIError::NotFound;
            else if (str == "OpenError")
                return LIError::OpenError;
            else if (str == "AccessDenied")
                return LIError::AccessDenied;
            else if (str == "ReadError")
                return LIError::ReadError;
            else if (str == "WriteError")
                return LIError::WriteError;
            else if (str == "PermissionDenied")
                return LIError::PermissionDenied;
            else if (str == "ParseError")
                return LIError::ParseError;
            else if (str == "InvalidPath")
                return LIError::InvalidPath;
            else if (str == "FileExists")
                return LIError::FileExists;
            else if (str == "DirectoryNotEmpty")
                return LIError::DirectoryNotEmpty;
            else if (str == "TooManyOpenFiles")
                return LIError::TooManyOpenFiles;
            else if (str == "DiskFull")
                return LIError::DiskFull;
            else if (str == "LoadError")
                return LIError::LoadError;
            else if (str == "UnloadError")
                return LIError::UnLoadError;
            else
                throw std::runtime_error("Invalid LIError string: " + str);
        }));

    m->add(user_type<ErrorInfo>(), "ErrorInfo");
    m->add(Carbon::fun(&ErrorInfo::errorMessage), "errorMessage");
    m->add(Carbon::fun(&ErrorInfo::moduleName), "moduleName");
    m->add(Carbon::fun(&ErrorInfo::functionName), "functionName");
    m->add(Carbon::fun(&ErrorInfo::line), "line");
    m->add(Carbon::fun(&ErrorInfo::fileName), "fileName");
    m->add(Carbon::fun(&ErrorInfo::timestamp), "timestamp");
    m->add(Carbon::fun(&ErrorInfo::uuid), "uuid");

    m->add(user_type<ErrorStack>(), "ErrorStack");
    m->add(Carbon::fun(&ErrorStack::createShared), "create_error_stack");
    m->add(Carbon::fun(&ErrorStack::createUnique), "create_unique_error_stack");
    m->add(Carbon::fun(&ErrorStack::insertError), "insert_error");
    m->add(Carbon::fun(&ErrorStack::getFilteredErrorsByModule),
           "get_filtered_errors_by_module");
    m->add(Carbon::fun(&ErrorStack::getCompressedErrors),
           "get_compressed_errors");
    m->add(Carbon::fun(&ErrorStack::setFilteredModules),
           "set_filtered_modules");
    m->add(Carbon::fun(&ErrorStack::clearFilteredModules),
           "clear_filtered_modules");
    m->add(Carbon::fun(&ErrorStack::printFilteredErrorStack),
           "print_filtered_error_stack");

    return m;
}
}  // namespace Atom::_Script::Error

#endif
