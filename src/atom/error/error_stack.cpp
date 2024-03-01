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

#include <ctime>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/time.hpp"

namespace Atom::Error
{
    std::ostream &operator<<(std::ostream &os, const ErrorInfo &error)
    {
        os << "{"
           << "\"errorMessage\": \"" << error.errorMessage << "\","
           << "\"moduleName\": \"" << error.moduleName << "\","
           << "\"functionName\": \"" << error.functionName << "\","
           << "\"line\": " << error.line << ","
           << "\"fileName\": \"" << error.fileName << "\","
           << "\"timestamp\": \"" << Atom::Utils::timeStampTostd::string(error.timestamp) << "\","
           << "\"uuid\": \"" << error.uuid << "\""
           << "}";

        return os;
    }

    std::string operator<<(const std::string &str, const ErrorInfo &error)
    {
        std::stringstream ss;
        ss << "{"
           << "\"errorMessage\": \"" << error.errorMessage << "\","
           << "\"moduleName\": \"" << error.moduleName << "\","
           << "\"functionName\": \"" << error.functionName << "\","
           << "\"line\": " << error.line << ","
           << "\"fileName\": \"" << error.fileName << "\","
           << "\"timestamp\": \"" << Atom::Utils::timeStampTostd::string(error.timestamp) << "\","
           << "\"uuid\": \"" << error.uuid << "\""
           << "}";

        return ss.str();
    }

    ErrorStack::ErrorStack()
    {
    }

    std::shared_ptr<ErrorStack> ErrorStack::createShared()
    {
        return std::make_shared<ErrorStack>();
    }

    std::unique_ptr<ErrorStack> ErrorStack::createUnique()
    {
        return std::make_unique<ErrorStack>();
    }

    void ErrorStack::insertError(const std::string &errorMessage, const std::string &moduleName, const std::string &functionName, int line, const std::string &fileName)
    {
        auto currentTime = std::time(nullptr);

        auto iter = std::find_if(errorStack.begin(), errorStack.end(),
                                 [&errorMessage, &moduleName](const ErrorInfo &error)
                                 {
                                     return error.errorMessage == errorMessage && error.moduleName == moduleName;
                                 });

        if (iter != errorStack.end())
        {
            iter->timestamp = currentTime;
        }
        else
        {
            errorStack.emplace_back(ErrorInfo{errorMessage, moduleName, functionName, line, fileName, currentTime, ""});
        }

        updateCompressedErrors();
    }

    void ErrorStack::setFilteredModules(const std::vector<std::string> &modules)
    {
        filteredModules = modules;
    }

    void ErrorStack::clearFilteredModules()
    {
        filteredModules.clear();
    }

    void ErrorStack::printFilteredErrorStack() const
    {
        for (const auto &error : errorStack)
        {
            if (std::find(filteredModules.begin(), filteredModules.end(), error.moduleName) == filteredModules.end())
            {
                LOG_F(ERROR, "{}", error.errorMessage);
            }
        }
    }

    std::vector<ErrorInfo> ErrorStack::getFilteredErrorsByModule(const std::string &moduleName) const
    {
        std::vector<ErrorInfo> errors;

        std::copy_if(errorStack.begin(), errorStack.end(), std::back_inserter(errors),
                     [&moduleName, this](const ErrorInfo &error)
                     {
                         return error.moduleName == moduleName && std::find(filteredModules.begin(), filteredModules.end(), error.moduleName) == filteredModules.end();
                     });

        return errors;
    }

    std::string ErrorStack::getCompressedErrors() const
    {
        std::stringstream compressedErrors;

        for (const auto &error : compressedErrorStack)
        {
            compressedErrors << error.errorMessage << " ";
        }

        return compressedErrors.str();
    }

    void ErrorStack::updateCompressedErrors()
    {
        compressedErrorStack.clear();

        for (const auto &error : errorStack)
        {
            auto iter = std::find_if(compressedErrorStack.begin(), compressedErrorStack.end(),
                                     [&error](const ErrorInfo &compressedError)
                                     {
                                         return compressedError.errorMessage == error.errorMessage && compressedError.moduleName == error.moduleName;
                                     });

            if (iter != compressedErrorStack.end())
            {
                iter->timestamp = error.timestamp;
            }
            else
            {
                compressedErrorStack.push_back(error);
            }
        }

        sortCompressedErrorStack();
    }

    void ErrorStack::sortCompressedErrorStack()
    {
        std::sort(compressedErrorStack.begin(), compressedErrorStack.end(),
                  [](const ErrorInfo &error1, const ErrorInfo &error2)
                  {
                      return error1.timestamp > error2.timestamp;
                  });
    }

}
