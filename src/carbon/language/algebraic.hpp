

#ifndef CARBON_ALGEBRAIC_HPP
#define CARBON_ALGEBRAIC_HPP

#include "../utils/hash.hpp"

#include <string>

namespace Carbon {
struct Operators {
    enum class Opers {
        equals,
        less_than,
        greater_than,
        less_than_equal,
        greater_than_equal,
        not_equal,
        assign,
        pre_increment,
        pre_decrement,
        assign_product,
        assign_sum,
        assign_quotient,
        assign_difference,
        assign_bitwise_and,
        assign_bitwise_or,
        assign_shift_left,
        assign_shift_right,
        assign_remainder,
        assign_bitwise_xor,
        shift_left,
        shift_right,
        remainder,
        bitwise_and,
        bitwise_or,
        bitwise_xor,
        bitwise_complement,
        sum,
        quotient,
        product,
        difference,
        unary_plus,
        unary_minus,
        invalid
    };

    constexpr static const char *to_string(Opers t_oper) noexcept {
        constexpr const char *opers[] = {
            "",   "==", "<",  ">",  "<=", ">=", "!=", "",   "=",   "++",
            "--", "*=", "+=", "/=", "-=", "",   "&=", "|=", "<<=", ">>=",
            "%=", "^=", "",   "<<", ">>", "%",  "&",  "|",  "^",   "~",
            "",   "+",  "/",  "*",  "-",  "+",  "-",  ""};
        return opers[static_cast<int>(t_oper)];
    }

    constexpr static Opers to_operator(std::string_view t_str,
                                       bool t_is_unary = false) noexcept {
#ifdef CARBON_MSVC
#pragma warning(push)
#pragma warning(disable : 4307)
#endif

        const auto op_hash = utility::hash(t_str);
        switch (op_hash) {
            case utility::hash("=="): {
                return Opers::equals;
            }
            case utility::hash("<"): {
                return Opers::less_than;
            }
            case utility::hash(">"): {
                return Opers::greater_than;
            }
            case utility::hash("<="): {
                return Opers::less_than_equal;
            }
            case utility::hash(">="): {
                return Opers::greater_than_equal;
            }
            case utility::hash("!="): {
                return Opers::not_equal;
            }
            case utility::hash("="): {
                return Opers::assign;
            }
            case utility::hash("++"): {
                return Opers::pre_increment;
            }
            case utility::hash("--"): {
                return Opers::pre_decrement;
            }
            case utility::hash("*="): {
                return Opers::assign_product;
            }
            case utility::hash("+="): {
                return Opers::assign_sum;
            }
            case utility::hash("-="): {
                return Opers::assign_difference;
            }
            case utility::hash("&="): {
                return Opers::assign_bitwise_and;
            }
            case utility::hash("|="): {
                return Opers::assign_bitwise_or;
            }
            case utility::hash("<<="): {
                return Opers::assign_shift_left;
            }
            case utility::hash(">>="): {
                return Opers::assign_shift_right;
            }
            case utility::hash("%="): {
                return Opers::assign_remainder;
            }
            case utility::hash("^="): {
                return Opers::assign_bitwise_xor;
            }
            case utility::hash("<<"): {
                return Opers::shift_left;
            }
            case utility::hash(">>"): {
                return Opers::shift_right;
            }
            case utility::hash("%"): {
                return Opers::remainder;
            }
            case utility::hash("&"): {
                return Opers::bitwise_and;
            }
            case utility::hash("|"): {
                return Opers::bitwise_or;
            }
            case utility::hash("^"): {
                return Opers::bitwise_xor;
            }
            case utility::hash("~"): {
                return Opers::bitwise_complement;
            }
            case utility::hash("+"): {
                return t_is_unary ? Opers::unary_plus : Opers::sum;
            }
            case utility::hash("-"): {
                return t_is_unary ? Opers::unary_minus : Opers::difference;
            }
            case utility::hash("/"): {
                return Opers::quotient;
            }
            case utility::hash("*"): {
                return Opers::product;
            }
            default: {
                return Opers::invalid;
            }
        }
#ifdef CARBON_MSVC
#pragma warning(pop)
#endif
    }
};
}  // namespace Carbon

#endif /* _CARBON_ALGEBRAIC_HPP */
