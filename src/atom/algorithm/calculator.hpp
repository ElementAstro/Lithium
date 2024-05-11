/*
 * calculator.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: A calculator implementation under C++20

**************************************************/

#ifndef ATOM_ALGORITHM_CALCULATOR_HPP
#define ATOM_ALGORITHM_CALCULATOR_HPP

#include <cctype>
#include <cmath>
#include <cstddef>
#include <functional>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace atom::algorithm {
class error : public std::runtime_error {
public:
    error(std::string_view expr, std::string_view message)
        : std::runtime_error(std::string(message)), expr_(expr) {}
    std::string_view expression() const noexcept { return expr_; }

private:
    std::string expr_;
};

template <typename T>
class ExpressionParser {
public:
    T eval(std::string_view expr) {
        T result = 0;
        index_ = 0;
        expr_ = expr;
        try {
            result = parseExpr();
            if (!isEnd())
                unexpected();
        } catch (const error &) {
            stack_ = {};
            throw;
        }
        return result;
    }

    T eval(char c) { return eval(std::string_view(&c, 1)); }

    void set(const std::string &name, T value) { variables_[name] = value; }

    void set(const std::string &name, std::function<T(T)> func) {
        functions_[name] = func;
    }

private:
    std::unordered_map<std::string, T> variables_;
    std::unordered_map<std::string, std::function<T(T)>> functions_;

    enum {
        OPERATOR_NULL,
        OPERATOR_BITWISE_OR,      /// |
        OPERATOR_BITWISE_XOR,     /// ^
        OPERATOR_BITWISE_AND,     /// &
        OPERATOR_BITWISE_SHL,     /// <<
        OPERATOR_BITWISE_SHR,     /// >>
        OPERATOR_ADDITION,        /// +
        OPERATOR_SUBTRACTION,     /// -
        OPERATOR_MULTIPLICATION,  /// *
        OPERATOR_DIVISION,        /// /
        OPERATOR_MODULO,          /// %
        OPERATOR_POWER,           /// **
        OPERATOR_EXPONENT         /// e, E
    };

    struct Operator {
        int op;
        int precedence;
        char associativity;
        constexpr Operator(int opr, int prec, char assoc) noexcept
            : op(opr), precedence(prec), associativity(assoc) {}
    };

    struct OperatorValue {
        Operator op;
        T value;
        constexpr OperatorValue(const Operator &opr, T val) noexcept
            : op(opr), value(val) {}
        constexpr int getPrecedence() const noexcept { return op.precedence; }
        constexpr bool isNull() const noexcept {
            return op.op == OPERATOR_NULL;
        }
    };

    static T pow(T x, T n) {
        T res = 1;

        while (n > 0) {
            if (n & 1) {
                res *= x;
            }

            x *= x;
            n >>= 1;
        }

        return res;
    }

    [[nodiscard]] T checkZero(T value) const {
        if (value == 0) {
            auto divOperators = std::string_view("/%");
            auto division = expr_.find_last_of(divOperators, index_ - 2);
            std::ostringstream msg;
            msg << "Parser error: division by 0";
            if (division != std::string_view::npos)
                msg << " (error token is \""
                    << expr_.substr(division, expr_.size() - division) << "\")";
            throw error(expr_, msg.str());
        }
        return value;
    }

    [[nodiscard]] constexpr T calculate(T v1, T v2,
                                        const Operator &op) const noexcept {
        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            switch (op.op) {
                case OPERATOR_ADDITION:
                    return v1 + v2;
                case OPERATOR_SUBTRACTION:
                    return v1 - v2;
                case OPERATOR_MULTIPLICATION:
                    return v1 * v2;
                case OPERATOR_DIVISION:
                    return v1 / checkZero(v2);
                case OPERATOR_POWER:
                    return std::pow(v1, v2);
                case OPERATOR_EXPONENT:
                    return v1 * std::pow(10, v2);
                default:
                    return 0;
            }
        } else {
            switch (op.op) {
                case OPERATOR_BITWISE_OR:
                    return v1 | v2;
                case OPERATOR_BITWISE_XOR:
                    return v1 ^ v2;
                case OPERATOR_BITWISE_AND:
                    return v1 & v2;
                case OPERATOR_BITWISE_SHL:
                    return v1 << v2;
                case OPERATOR_BITWISE_SHR:
                    return v1 >> v2;
                case OPERATOR_ADDITION:
                    return v1 + v2;
                case OPERATOR_SUBTRACTION:
                    return v1 - v2;
                case OPERATOR_MULTIPLICATION:
                    return v1 * v2;
                case OPERATOR_DIVISION:
                    return v1 / checkZero(v2);
                case OPERATOR_MODULO:
                    return v1 % checkZero(v2);
                case OPERATOR_POWER:
                    return pow(v1, v2);
                case OPERATOR_EXPONENT:
                    return v1 * pow(10, v2);
                default:
                    return 0;
            }
        }
    }

    [[nodiscard]] constexpr bool isEnd() const noexcept {
        return index_ >= expr_.size();
    }

    [[nodiscard]] constexpr char getCharacter() const noexcept {
        if (!isEnd())
            return expr_[index_];
        return 0;
    }

    void expect(std::string_view str) {
        if (expr_.substr(index_, str.size()) != str)
            unexpected();
        index_ += str.size();
    }

    [[noreturn]] void unexpected() const {
        std::ostringstream msg;
        msg << "Syntax error: unexpected token \"" << expr_.substr(index_)
            << "\" at index " << index_;
        throw error(expr_, msg.str());
    }

    constexpr void eatSpaces() noexcept {
        while (std::isspace(getCharacter()) != 0)
            index_++;
    }

    [[nodiscard]] Operator parseOp() {
        eatSpaces();
        switch (std::tolower(getCharacter())) {
            case '|':
                index_++;
                return Operator(OPERATOR_BITWISE_OR, 4, 'L');
            case '^':
                index_++;
                return Operator(OPERATOR_BITWISE_XOR, 5, 'L');
            case '&':
                index_++;
                return Operator(OPERATOR_BITWISE_AND, 6, 'L');
            case '<':
                expect("<<");
                return Operator(OPERATOR_BITWISE_SHL, 9, 'L');
            case '>':
                expect(">>");
                return Operator(OPERATOR_BITWISE_SHR, 9, 'L');
            case '+':
                index_++;
                return Operator(OPERATOR_ADDITION, 10, 'L');
            case '-':
                index_++;
                return Operator(OPERATOR_SUBTRACTION, 10, 'L');
            case '/':
                index_++;
                return Operator(OPERATOR_DIVISION, 20, 'L');
            case '%':
                index_++;
                return Operator(OPERATOR_MODULO, 20, 'L');
            case '*':
                index_++;
                if (getCharacter() != '*')
                    return Operator(OPERATOR_MULTIPLICATION, 20, 'L');
                index_++;
                return Operator(OPERATOR_POWER, 30, 'R');
            case 'e':
                index_++;
                return Operator(OPERATOR_EXPONENT, 40, 'R');
            default:
                return Operator(OPERATOR_NULL, 0, 'L');
        }
    }

    [[nodiscard]] static constexpr T toInteger(char c) noexcept {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 0xa;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 0xa;
        return 0xf + 1;
    }

    [[nodiscard]] constexpr T getInteger() const noexcept {
        return toInteger(getCharacter());
    }

    [[nodiscard]] T parseDecimal() {
        T value = 0;
        T fraction = 1;
        bool decimal_point = false;
        std::size_t length = 0;
        std::size_t max_length = std::numeric_limits<T>::digits10 + 1;

        while (index_ < expr_.size()) {
            char c = expr_[index_];
            if (std::isdigit(c)) {
                if (length >= max_length) {
                    throw error(expr_, "Number too large");
                }
                value = value * 10 + (c - '0');
                if (decimal_point)
                    fraction *= 10;
                length++;
            } else if (c == '.' && !decimal_point) {
                if (std::is_integral<T>::value) {
                    throw error(
                        expr_,
                        "Decimal numbers are not allowed in integer mode");
                }
                decimal_point = true;
            } else if (c == '.' && decimal_point) {
                throw error(expr_, "Multiple decimal points in number");
            } else {
                break;
            }
            index_++;
        }
        return decimal_point ? value / fraction : value;
    }

    [[nodiscard]] T parseHex() {
        index_ += 2;
        T value = 0;
        std::size_t length = 0;
        std::size_t max_length = std::numeric_limits<T>::digits / 4;

        while (index_ < expr_.size()) {
            T h = getInteger();
            if (h <= 0xf) {
                if (length >= max_length) {
                    throw error(expr_, "Number too large");
                }
                value = value * 0x10 + h;
                length++;
                index_++;
            } else {
                break;
            }
        }
        return value;
    }

    [[nodiscard]] constexpr bool isHex() const noexcept {
        if (index_ + 2 < expr_.size()) {
            char x = expr_[index_ + 1];
            char h = expr_[index_ + 2];
            return (std::tolower(x) == 'x' && toInteger(h) <= 0xf);
        }
        return false;
    }

    [[nodiscard]] T parseValue() {
        T val = 0;
        eatSpaces();
        if (std::isalpha(getCharacter())) {
            std::string name;
            while (std::isalnum(getCharacter()) || getCharacter() == '_') {
                name += getCharacter();
                index_++;
            }
            if (functions_.count(name) > 0) {
                eatSpaces();
                if (getCharacter() != '(') {
                    throw error(expr_, "Expected '(' after function name");
                }
                index_++;
                T arg = parseExpr();
                if (getCharacter() != ')') {
                    throw error(expr_, "Expected ')' after function argument");
                }
                index_++;
                val = functions_[name](arg);
            } else if (variables_.count(name) > 0) {
                val = variables_[name];
            } else {
                throw error(expr_, "Undefined function or variable: " + name);
            }
        } else {
            switch (getCharacter()) {
                case '0':
                    if (isHex())
                        val = parseHex();
                    else
                        val = parseDecimal();
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    val = parseDecimal();
                    break;
                case '(':
                    index_++;
                    val = parseExpr();
                    eatSpaces();
                    if (getCharacter() != ')') {
                        if (!isEnd())
                            unexpected();
                        throw error(
                            expr_,
                            "Syntax error: `)' expected at end of expression");
                    }
                    index_++;
                    break;
                case '~':
                    if constexpr (std::is_same_v<T, int>) {
                        index_++;
                        val = ~parseValue();
                    } else {
                        throw error(expr_,
                                    "Syntax error: `~' not supported for "
                                    "non-int types");
                    }
                    break;
                case '+':
                    index_++;
                    val = parseValue();
                    break;
                case '-':
                    index_++;
                    val = -parseValue();
                    break;
                default:
                    if (!isEnd())
                        unexpected();
                    throw error(
                        expr_,
                        "Syntax error: value expected at end of expression");
            }
        }

        return val;
    }

    T parseExpr() {
        stack_.emplace(Operator(OPERATOR_NULL, 0, 'L'), 0);
        T value = parseValue();

        while (!stack_.empty()) {
            Operator op(parseOp());
            while (op.precedence < stack_.top().getPrecedence() ||
                   (op.precedence == stack_.top().getPrecedence() &&
                    op.associativity == 'L')) {
                if (stack_.top().isNull()) {
                    stack_.pop();
                    return value;
                }
                value = calculate(stack_.top().value, value, stack_.top().op);
                stack_.pop();
            }

            stack_.emplace(op, value);
            value = parseValue();
        }
        return 0;
    }

    std::string_view expr_;
    std::size_t index_ = 0;
    std::stack<OperatorValue> stack_;
};

template <typename T>
inline T eval(std::string_view expression) {
    ExpressionParser<T> parser;
    return parser.eval(expression);
}

inline double eval(const std::string &expression) {
    return eval<double>(expression);
}

}  // namespace atom::algorithm

#endif