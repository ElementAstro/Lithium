/*
 * error_stack.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Error Stack

**************************************************/

#include "error_stack.hpp"

#include <algorithm>
#include <ctime>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/time.hpp"

namespace atom::error {
std::ostream &operator<<(std::ostream &os, const ErrorInfo &error) {
    os << "{" << R"("errorMessage": ")" << error.errorMessage << "\","
       << R"("moduleName": ")" << error.moduleName << "\","
       << R"("functionName": ")" << error.functionName << "\","
       << "\"line\": " << error.line << "," << R"("fileName": ")"
       << error.fileName << "\"," << "\"timestamp\": \""
       << atom::utils::timeStampToString(error.timestamp) << "\","
       << "\"uuid\": \"" << error.uuid << "\"" << "}";

    return os;
}

std::string operator<<([[maybe_unused]] const std::string &str,
                       const ErrorInfo &error) {
    std::stringstream ss;
    ss << "{" << R"("errorMessage": ")" << error.errorMessage << "\","
       << R"("moduleName": ")" << error.moduleName << "\","
       << R"("functionName": ")" << error.functionName << "\","
       << "\"line\": " << error.line << "," << R"("fileName": ")"
       << error.fileName << "\"," << R"("timestamp": ")"
       << atom::utils::timeStampToString(error.timestamp) << "\","
       << R"("uuid": ")" << error.uuid << "\"" << "}";

    return ss.str();
}

auto ErrorStack::createShared() -> std::shared_ptr<ErrorStack> {
    return std::make_shared<ErrorStack>();
}

auto ErrorStack::createUnique() -> std::unique_ptr<ErrorStack> {
    return std::make_unique<ErrorStack>();
}

void ErrorStack::insertError(const std::string &errorMessage,
                             const std::string &moduleName,
                             const std::string &functionName, int line,
                             const std::string &fileName) {
    auto currentTime = std::time(nullptr);

    auto iter =
        std::find_if(errorStack_.begin(), errorStack_.end(),
                     [&errorMessage, &moduleName](const ErrorInfo &error) {
                         return error.errorMessage == errorMessage &&
                                error.moduleName == moduleName;
                     });

    if (iter != errorStack_.end()) {
        iter->timestamp = currentTime;
    } else {
        errorStack_.emplace_back(ErrorInfo{errorMessage, moduleName,
                                           functionName, line, fileName,
                                           currentTime, ""});
    }

    updateCompressedErrors();
}

void ErrorStack::setFilteredModules(const std::vector<std::string> &modules) {
    filteredModules_ = modules;
}

void ErrorStack::clearFilteredModules() { filteredModules_.clear(); }

void ErrorStack::printFilteredErrorStack() const {
    for (const auto &error : errorStack_) {
        if (std::find(filteredModules_.begin(), filteredModules_.end(),
                      error.moduleName) == filteredModules_.end()) {
            LOG_F(ERROR, "{}", error.errorMessage);
        }
    }
}

auto ErrorStack::getFilteredErrorsByModule(const std::string &moduleName) const
    -> std::vector<ErrorInfo> {
    std::vector<ErrorInfo> errors;

    std::copy_if(
        errorStack_.begin(), errorStack_.end(), std::back_inserter(errors),
        [&moduleName, this](const ErrorInfo &error) {
            return error.moduleName == moduleName &&
                   std::find(filteredModules_.begin(), filteredModules_.end(),
                             error.moduleName) == filteredModules_.end();
        });

    return errors;
}

auto ErrorStack::getCompressedErrors() const -> std::string {
    std::stringstream compressedErrors;

    for (const auto &error : compressedErrorStack_) {
        compressedErrors << error.errorMessage << " ";
    }

    return compressedErrors.str();
}

void ErrorStack::updateCompressedErrors() {
    compressedErrorStack_.clear();

    for (const auto &error : errorStack_) {
        auto iter = std::find_if(
            compressedErrorStack_.begin(), compressedErrorStack_.end(),
            [&error](const ErrorInfo &compressedError) {
                return compressedError.errorMessage == error.errorMessage &&
                       compressedError.moduleName == error.moduleName;
            });

        if (iter != compressedErrorStack_.end()) {
            iter->timestamp = error.timestamp;
        } else {
            compressedErrorStack_.push_back(error);
        }
    }

    sortCompressedErrorStack();
}

void ErrorStack::sortCompressedErrorStack() {
    std::sort(compressedErrorStack_.begin(), compressedErrorStack_.end(),
              [](const ErrorInfo &error1, const ErrorInfo &error2) {
                  return error1.timestamp > error2.timestamp;
              });
}

}  // namespace atom::error
