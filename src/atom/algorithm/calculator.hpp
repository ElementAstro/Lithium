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
#include <cstddef>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>

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
        } catch (const calculator::error&) {
            stack_ = {};
            throw;
        }
        return result;
    }

    T eval(char c) { return eval(std::string_view(&c, 1)); }

private:
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
        constexpr OperatorValue(const Operator& opr, T val) noexcept
            : op(opr), value(val) {}
        constexpr int getPrecedence() const noexcept { return op.precedence; }
        constexpr bool isNull() const noexcept {
            return op.op == OPERATOR_NULL;
        }
    };

    static T pow(T x, T n) {
        T res = 1;

        while (n > 0) {
            if (n % 2 != 0) {
                res *= x;
                n -= 1;
            }
            n /= 2;

            if (n > 0)
                x *= x;
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
            throw calculator::error(expr_, msg.str());
        }
        return value;
    }

    [[nodiscard]] constexpr T calculate(T v1, T v2,
                                        const Operator& op) const noexcept {
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
        throw calculator::error(expr_, msg.str());
    }

    constexpr void eatSpaces() noexcept {
        while (std::isspace(getCharacter()) != 0)
            index_++;
    }

    [[nodiscard]] Operator parseOp() {
        eatSpaces();
        switch (getCharacter()) {
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
            case 'E':
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
        for (T d; (d = getInteger()) <= 9; index_++)
            value = value * 10 + d;
        return value;
    }

    [[nodiscard]] T parseHex() {
        index_ += 2;
        T value = 0;
        for (T h; (h = getInteger()) <= 0xf; index_++)
            value = value * 0x10 + h;
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
                    throw calculator::error(
                        expr_,
                        "Syntax error: `)' expected at end of expression");
                }
                index_++;
                break;
            case '~':
                index_++;
                val = ~parseValue();
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
                throw calculator::error(
                    expr_, "Syntax error: value expected at end of expression");
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

inline int eval(const std::string& expression) { return eval<int>(expression); }

}  // namespace atom::algorithm

#endif