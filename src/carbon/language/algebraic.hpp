

#ifndef CARBON_ALGEBRAIC_HPP
#define CARBON_ALGEBRAIC_HPP

#include "atom/algorithm/hash.hpp"

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

        const auto op_hash = Atom::Algorithm::fnv1a_hash(t_str);
        switch (op_hash) {
            case Atom::Algorithm::fnv1a_hash("=="): {
                return Opers::equals;
            }
            case Atom::Algorithm::fnv1a_hash("<"): {
                return Opers::less_than;
            }
            case Atom::Algorithm::fnv1a_hash(">"): {
                return Opers::greater_than;
            }
            case Atom::Algorithm::fnv1a_hash("<="): {
                return Opers::less_than_equal;
            }
            case Atom::Algorithm::fnv1a_hash(">="): {
                return Opers::greater_than_equal;
            }
            case Atom::Algorithm::fnv1a_hash("!="): {
                return Opers::not_equal;
            }
            case Atom::Algorithm::fnv1a_hash("="): {
                return Opers::assign;
            }
            case Atom::Algorithm::fnv1a_hash("++"): {
                return Opers::pre_increment;
            }
            case Atom::Algorithm::fnv1a_hash("--"): {
                return Opers::pre_decrement;
            }
            case Atom::Algorithm::fnv1a_hash("*="): {
                return Opers::assign_product;
            }
            case Atom::Algorithm::fnv1a_hash("+="): {
                return Opers::assign_sum;
            }
            case Atom::Algorithm::fnv1a_hash("-="): {
                return Opers::assign_difference;
            }
            case Atom::Algorithm::fnv1a_hash("&="): {
                return Opers::assign_bitwise_and;
            }
            case Atom::Algorithm::fnv1a_hash("|="): {
                return Opers::assign_bitwise_or;
            }
            case Atom::Algorithm::fnv1a_hash("<<="): {
                return Opers::assign_shift_left;
            }
            case Atom::Algorithm::fnv1a_hash(">>="): {
                return Opers::assign_shift_right;
            }
            case Atom::Algorithm::fnv1a_hash("%="): {
                return Opers::assign_remainder;
            }
            case Atom::Algorithm::fnv1a_hash("^="): {
                return Opers::assign_bitwise_xor;
            }
            case Atom::Algorithm::fnv1a_hash("<<"): {
                return Opers::shift_left;
            }
            case Atom::Algorithm::fnv1a_hash(">>"): {
                return Opers::shift_right;
            }
            case Atom::Algorithm::fnv1a_hash("%"): {
                return Opers::remainder;
            }
            case Atom::Algorithm::fnv1a_hash("&"): {
                return Opers::bitwise_and;
            }
            case Atom::Algorithm::fnv1a_hash("|"): {
                return Opers::bitwise_or;
            }
            case Atom::Algorithm::fnv1a_hash("^"): {
                return Opers::bitwise_xor;
            }
            case Atom::Algorithm::fnv1a_hash("~"): {
                return Opers::bitwise_complement;
            }
            case Atom::Algorithm::fnv1a_hash("+"): {
                return t_is_unary ? Opers::unary_plus : Opers::sum;
            }
            case Atom::Algorithm::fnv1a_hash("-"): {
                return t_is_unary ? Opers::unary_minus : Opers::difference;
            }
            case Atom::Algorithm::fnv1a_hash("/"): {
                return Opers::quotient;
            }
            case Atom::Algorithm::fnv1a_hash("*"): {
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
