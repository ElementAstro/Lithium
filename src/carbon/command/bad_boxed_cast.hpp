

#ifndef CARBON_BAD_BOXED_CAST_HPP
#define CARBON_BAD_BOXED_CAST_HPP

#include <string>
#include <typeinfo>

#include "../defines.hpp"
#include "../utils/static_string.hpp"
#include "type_info.hpp"

namespace Carbon {
class Type_Info;
}  // namespace Carbon

namespace Carbon {
namespace exception {
/// \brief Thrown in the event that a Boxed_Value cannot be cast to the desired
/// type
///
/// It is used internally during function dispatch and may be used by the end
/// user.
///
/// \sa Carbon::boxed_cast
class bad_boxed_cast : public std::bad_cast {
public:
    bad_boxed_cast(Type_Info t_from, const std::type_info &t_to,
                   utility::Static_String t_what) noexcept
        : from(t_from), to(&t_to), m_what(std::move(t_what)) {}

    bad_boxed_cast(Type_Info t_from, const std::type_info &t_to) noexcept
        : from(t_from), to(&t_to), m_what("Cannot perform boxed_cast") {}

    explicit bad_boxed_cast(utility::Static_String t_what) noexcept
        : m_what(std::move(t_what)) {}

    bad_boxed_cast(const bad_boxed_cast &) noexcept = default;
    ~bad_boxed_cast() noexcept override = default;

    /// \brief Description of what error occurred
    const char *what() const noexcept override { return m_what.c_str(); }

    Type_Info from;  ///< Type_Info contained in the Boxed_Value
    const std::type_info *to =
        nullptr;  ///< std::type_info of the desired (but failed) result type

private:
    utility::Static_String m_what;
};
}  // namespace exception
}  // namespace Carbon

#endif
