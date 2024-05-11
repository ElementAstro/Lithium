/*
 * stepper.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Proxy Function Sequence

**************************************************/

#ifndef ATOM_FUNCTION_SEQUENCE_HPP
#define ATOM_FUNCTION_SEQUENCE_HPP

#include "atom/error/exception.hpp"
#include "proxy.hpp"

class FunctionSequence {
public:
    using FunctionType = std::function<std::any(const std::vector<std::any>&)>;

    // Register a function to be part of the sequence
    void register_function(FunctionType func) {
        functions.emplace_back(std::move(func));
    }

    // Run the last function with each set of arguments provided
    std::vector<std::any> run(
        const std::vector<std::vector<std::any>>& args_batch) {
        std::vector<std::any> results;
        if (functions.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return results;
        }

        try {
            auto& func = functions.back();
            for (const auto& args : args_batch) {
                results.emplace_back(func(args));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return results;
    }

    // Run all functions with each set of arguments provided and return the
    // results of each function
    std::vector<std::vector<std::any>> run_all(
        const std::vector<std::vector<std::any>>& args_batch) {
        std::vector<std::vector<std::any>> results_batch;
        if (functions.empty()) {
            THROW_EXCEPTION("No functions registered in the sequence");
            return results_batch;
        }

        try {
            for (const auto& args : args_batch) {
                std::vector<std::any> results;
                for (const auto& func : functions) {
                    results.emplace_back(func(args));
                }
                results_batch.emplace_back(std::move(results));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION(std::string("Exception caught: ") + e.what());
        }
        return results_batch;
    }

private:
    std::vector<FunctionType> functions;
};

#endif  // ATOM_FUNCTION_SEQUENCE_HPP
