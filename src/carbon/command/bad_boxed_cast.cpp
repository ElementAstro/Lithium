#include "bad_boxed_cast.hpp"

namespace Carbon::exception {
bad_boxed_cast::bad_boxed_cast(Type_Info t_from, const std::type_info &t_to,
                               utility::Static_String t_what)
    : from(t_from), to(&t_to), m_what(std::move(t_what)) {}

bad_boxed_cast::bad_boxed_cast(Type_Info t_from, const std::type_info &t_to)
    : from(t_from), to(&t_to), m_what("Cannot perform boxed_cast") {}

bad_boxed_cast::bad_boxed_cast(utility::Static_String t_what)
    : m_what(std::move(t_what)) {}

/// \brief Description of what error occurred
const char *bad_boxed_cast::what() const { return m_what.c_str(); }
}  // namespace Carbon::exception
