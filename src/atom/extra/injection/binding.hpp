#pragma once

#include "common.hpp"
#include "resolver.hpp"

namespace atom::extra {

template <typename T, typename... SymbolTypes>
class BindingScope {
public:
    void inTransientScope() {
        lifecycle_ = Lifecycle::Transient;
    }

    void inSingletonScope() {
        lifecycle_ = Lifecycle::Singleton;
        resolver_ = std::make_shared<CachedResolver<T, SymbolTypes...>>(resolver_);
    }

    void inRequestScope() {
        lifecycle_ = Lifecycle::Request;
    }

protected:
    ResolverPtr<T, SymbolTypes...> resolver_;
    Lifecycle lifecycle_ = Lifecycle::Transient;
};

template <typename T, typename... SymbolTypes>
class BindingTo : public BindingScope<T, SymbolTypes...> {
public:
    void toConstantValue(T&& value) {
        this->resolver_ = std::make_shared<ConstantResolver<T, SymbolTypes...>>(std::forward<T>(value));
    }

    BindingScope<T, SymbolTypes...>& toDynamicValue(Factory<T, SymbolTypes...>&& factory) {
        this->resolver_ = std::make_shared<DynamicResolver<T, SymbolTypes...>>(std::move(factory));
        return *this;
    }

    template <typename U>
    BindingScope<T, SymbolTypes...>& to() {
        this->resolver_ = std::make_shared<AutoResolver<T, U, SymbolTypes...>>();
        return *this;
    }
};

template <typename T, typename... SymbolTypes>
class Binding : public BindingTo<typename T::value, SymbolTypes...> {
public:
    typename T::value resolve(const Context<SymbolTypes...>& context) {
        if (!this->resolver_) {
            throw exceptions::ResolutionException("atom::extra::Resolver not found. Malformed binding.");
        }
        return this->resolver_->resolve(context);
    }

    void when(const Tag& tag) {
        tags_.push_back(tag);
    }

    void whenTargetNamed(const std::string& name) {
        targetName_ = name;
    }

    bool matchesTag(const Tag& tag) const {
        return std::find_if(tags_.begin(), tags_.end(),
                            [&](const Tag& t) { return t.name == tag.name; }) != tags_.end();
    }

    bool matchesTargetName(const std::string& name) const {
        return targetName_ == name;
    }

private:
    std::vector<Tag> tags_;
    std::string targetName_;
};

} // namespace atom::extra
