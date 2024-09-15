#ifndef ATOM_EXTRA_INICPP_INIFIELD_HPP
#define ATOM_EXTRA_INICPP_INIFIELD_HPP

#include "convert.hpp"

#include <string>
#include <utility>

namespace inicpp {

class IniField {
private:
    std::string value_;

public:
    IniField() = default;
    explicit IniField(std::string value) : value_(std::move(value)) {}
    IniField(const IniField &field) = default;
    ~IniField() = default;

    template <typename T>
    T as() const {
        Convert<T> conv;
        T result;
        conv.decode(value_, result);
        return result;
    }

    template <typename T>
    IniField &operator=(const T &value) {
        Convert<T> conv;
        conv.encode(value, value_);
        return *this;
    }

    IniField &operator=(const IniField &field) = default;
};

} // namespace inicpp

#endif // ATOM_EXTRA_INICPP_INIFIELD_HPP