/*!
 * \file stepper.hpp
 * \brief Proxy Function Sequence
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_SEQUENCE_HPP
#define ATOM_META_SEQUENCE_HPP

#include <any>
#include <functional>

#include "atom/error/exception.hpp"

namespace atom::meta {
class FunctionSequence {
public:
    using FunctionType = std::function<std::any(const std::vector<std::any>&)>;

    // Register a function to be part of the sequence
    void registerFunction(FunctionType func) {
        functions_.emplace_back(std::move(func));
    }

    // Run the last function with each set of arguments provided
    auto run(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::any> {
        std::vector<std::any> results;
        if (functions_.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return results;
        }

        try {
            auto& func = functions_.back();
            results.reserve(argsBatch.size());
            for (const auto& args : argsBatch) {
                results.emplace_back(func(args));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return results;
    }

    // Run all functions with each set of arguments provided and return the
    // results of each function
    auto runAll(const std::vector<std::vector<std::any>>& argsBatch)
        -> std::vector<std::vector<std::any>> {
        std::vector<std::vector<std::any>> resultsBatch;
        if (functions_.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return resultsBatch;
        }

        try {
            resultsBatch.reserve(argsBatch.size());
            for (const auto& args : argsBatch) {
                std::vector<std::any> results;
                results.reserve(functions_.size());
                for (const auto& func : functions_) {
                    results.emplace_back(func(args));
                }
                resultsBatch.emplace_back(std::move(results));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return resultsBatch;
    }

private:
    std::vector<FunctionType> functions_;
};
}  // namespace atom::meta

#endif  // ATOM_META_SEQUENCE_HPP