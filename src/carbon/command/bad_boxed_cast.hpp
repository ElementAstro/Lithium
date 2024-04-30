

#ifndef CARBON_BAD_BOXED_CAST_HPP
#define CARBON_BAD_BOXED_CAST_HPP

#include <string>
#include <typeinfo>

#include "../defines.hpp"
#include "atom/type/static_string.hpp"
#include "atom/function/type_info.hpp"

namespace Carbon {
namespace exception {
/**
 * @brief Exception thrown to indicate a failed cast operation involving
 * Boxed_Value.
 *
 * This exception is thrown when an attempt to cast a Boxed_Value to another
 * type fails. It provides information about the source type (from), the target
 * type (to), and a description of the error.
 */
class bad_boxed_cast : public std::bad_cast {
public:
    /**
     * @brief Constructs a bad_boxed_cast object with detailed information.
     *
     * @param t_from The Type_Info of the source type contained in the
     * Boxed_Value.
     * @param t_to The std::type_info of the desired (but failed) result type.
     * @param t_what A Static_String providing additional description of the
     * error.
     */
    bad_boxed_cast(Type_Info t_from, const std::type_info &t_to,
                   Static_String t_what) noexcept;

    /**
     * @brief Constructs a bad_boxed_cast object with type information only.
     *
     * @param t_from The Type_Info of the source type contained in the
     * Boxed_Value.
     * @param t_to The std::type_info of the desired (but failed) result type.
     */
    bad_boxed_cast(Type_Info t_from, const std::type_info &t_to) noexcept;

    /**
     * @brief Constructs a bad_boxed_cast object with a custom error message.
     *
     * @param t_what A Static_String providing additional description of the
     * error.
     */
    explicit bad_boxed_cast(Static_String t_what) noexcept;

    /**
     * @brief Destructor for bad_boxed_cast.
     */
    ~bad_boxed_cast() noexcept override = default;

    /**
     * @brief Retrieves a C-style string describing the error.
     *
     * @return A pointer to a constant C-style string describing the error.
     */
    const char *what() const noexcept override;

    Type_Info from;  ///< Type_Info contained in the Boxed_Value
    const std::type_info *to =
        nullptr;  ///< std::type_info of the desired (but failed) result type

private:
    Static_String m_what;  ///< Additional description of the error
};

}  // namespace exception
}  // namespace Carbon

#endif
