#ifndef ATOM_FUNCTION_STEPPER_HPP
#define ATOM_FUNCTION_STEPPER_HPP

#include "proxy.hpp"

#include "atom/error/exception.hpp"

template <typename Ret, typename... Args>
class FunctionChain {
public:
    void register_function(std::function<Ret(Args...)> func) {
        functions.emplace_back(std::move(func));
    }

    std::any run(const std::vector<std::any>& args) {
        try {
            std::any result;
            for (const auto& func : functions) {
                result = ProxyFunction<Ret, Args...>(func)(args);
            }
            return result;
        } catch (const std::exception& e) {
            THROW_EXCEPTION("Exception caught");
        }
    }

    std::vector<std::any> run_all(const std::vector<std::any>& args) {
        std::vector<std::any> results;
        try {
            for (const auto& func : functions) {
                results.emplace_back(ProxyFunction<Ret, Args...>(func)(args));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION("Exception caught");
        }
        return results;
    }

private:
    std::vector<std::function<Ret(Args...)>> functions;
};

template <typename Ret, typename... Args>
class FunctionStepper {
public:
    void register_function(std::function<Ret(Args...)> func) {
        functions.emplace_back(std::move(func));
    }

    std::any run(const std::vector<std::any>& args) {
        try {
            std::any result;
            for (const auto& func : functions) {
                result = ProxyFunction<Ret, Args...>(func)(args);
            }
            return result;
        } catch (const std::exception& e) {
            THROW_EXCEPTION("Exception caught");
        }
    }

    std::vector<std::any> run_all(const std::vector<std::any>& args) {
        std::vector<std::any> results;
        try {
            for (const auto& func : functions) {
                results.emplace_back(ProxyFunction<Ret, Args...>(func)(args));
            }
        } catch (const std::exception& e) {
            THROW_EXCEPTION("Exception caught");
        }
        return results;
    }

private:
    std::vector<std::function<Ret(Args...)>> functions;
};

#endif
