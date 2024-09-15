#ifndef ATOM_EXTRA_INICPP_INISECTION_HPP
#define ATOM_EXTRA_INICPP_INISECTION_HPP

#include <map>
#include <string>

#include "field.hpp"

namespace inicpp {

template <typename Comparator>
class IniSectionBase : public std::map<std::string, IniField, Comparator> {
public:
    IniSectionBase() = default;
    ~IniSectionBase() = default;
};

using IniSection = IniSectionBase<std::less<std::string>>;
using IniSectionCaseInsensitive = IniSectionBase<StringInsensitiveLess>;

}  // namespace inicpp

#endif  // ATOM_EXTRA_INICPP_INISECTION_HPP