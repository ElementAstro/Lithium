#pragma once

#include <optional>
#include "common.hpp"
#include "inject.hpp"

namespace atom::extra {

template <typename T, typename... SymbolTypes>
class Resolver {
public:
    virtual ~Resolver() = default;
    virtual T resolve(const Context<SymbolTypes...>&) = 0;
};

template <typename T, typename... SymbolTypes>
using ResolverPtr = std::shared_ptr<Resolver<T, SymbolTypes...>>;

template <typename T, typename... SymbolTypes>
class ConstantResolver : public Resolver<T, SymbolTypes...> {
public:
    explicit ConstantResolver(T value) : value_(std::move(value)) {}
    T resolve(const Context<SymbolTypes...>&) override { return value_; }

private:
    T value_;
};

template <typename T, typename... SymbolTypes>
class DynamicResolver : public Resolver<T, SymbolTypes...> {
public:
    explicit DynamicResolver(Factory<T, SymbolTypes...> factory)
        : factory_(std::move(factory)) {}
    T resolve(const Context<SymbolTypes...>& context) override {
        return factory_(context);
    }

private:
    Factory<T, SymbolTypes...> factory_;
};

template <typename T, typename U, typename... SymbolTypes>
class AutoResolver : public Resolver<T, SymbolTypes...> {
public:
    T resolve(const Context<SymbolTypes...>& context) override {
        return std::make_from_tuple<U>(
            InjectableA<U>::template resolve(context));
    }
};

template <typename T, typename U, typename... SymbolTypes>
class AutoResolver<std::unique_ptr<T>, U, SymbolTypes...>
    : public Resolver<std::unique_ptr<T>, SymbolTypes...> {
public:
    std::unique_ptr<T> resolve(
        const Context<SymbolTypes...>& context) override {
        return std::apply(
            [](auto&&... deps) {
                return std::make_unique<U>(
                    std::forward<decltype(deps)>(deps)...);
            },
            InjectableA<U>::template resolve(context));
    }
};

template <typename T, typename U, typename... SymbolTypes>
class AutoResolver<std::shared_ptr<T>, U, SymbolTypes...>
    : public Resolver<std::shared_ptr<T>, SymbolTypes...> {
public:
    std::shared_ptr<T> resolve(
        const Context<SymbolTypes...>& context) override {
        return std::apply(
            [](auto&&... deps) {
                return std::make_shared<U>(
                    std::forward<decltype(deps)>(deps)...);
            },
            InjectableA<U>::template resolve(context));
    }
};

template <typename T, typename... SymbolTypes>
class CachedResolver : public Resolver<T, SymbolTypes...> {
    static_assert(std::is_copy_constructible_v<T>,
                  "atom::extra::CachedResolver requires a copy constructor. Are "
                  "you caching a unique_ptr?");

public:
    explicit CachedResolver(ResolverPtr<T, SymbolTypes...> parent)
        : parent_(std::move(parent)) {}
    T resolve(const Context<SymbolTypes...>& context) override {
        if (!cached_.has_value()) {
            cached_ = parent_->resolve(context);
        }
        return cached_.value();
    }

private:
    std::optional<T> cached_;
    ResolverPtr<T, SymbolTypes...> parent_;
};

}  // namespace atom::extra