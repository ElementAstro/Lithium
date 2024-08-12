#include "bignumber.hpp"

#include <vector>

#include "atom/log/loguru.hpp"

namespace atom::algorithm {
auto BigNumber::add(const BigNumber& other) const -> BigNumber {
    if (isNegative() && other.isNegative()) {
        return negate().add(other.negate()).negate();
    }
    if (isNegative()) {
        return other.subtract(abs());
    }
    if (other.isNegative()) {
        return subtract(other.abs());
    }

    std::string result;
    int carry = 0;
    int i = numberString_.length() - 1;
    int j = other.numberString_.length() - 1;

    while (i >= 0 || j >= 0 || (carry != 0)) {
        int digit1 = (i >= 0) ? numberString_[i--] - '0' : 0;
        int digit2 = (j >= 0) ? other.numberString_[j--] - '0' : 0;
        int sum = digit1 + digit2 + carry;
        result.insert(result.begin(), '0' + (sum % 10));
        carry = sum / 10;
    }

    return {result};
}

auto BigNumber::subtract(const BigNumber& other) const -> BigNumber {
    if (isNegative() && other.isNegative()) {
        return other.negate().subtract(negate());
    }
    if (isNegative()) {
        return negate().add(other).negate();
    }
    if (other.isNegative()) {
        return add(other.negate());
    }
    if (*this < other) {
        return other.subtract(*this).negate();
    }

    std::string result;
    int carry = 0;
    int i = numberString_.length() - 1;
    int j = other.numberString_.length() - 1;

    while (i >= 0 || j >= 0) {
        int digit1 = (i >= 0) ? numberString_[i--] - '0' : 0;
        int digit2 = (j >= 0) ? other.numberString_[j--] - '0' : 0;
        int diff = digit1 - digit2 - carry;
        if (diff < 0) {
            diff += 10;
            carry = 1;
        } else {
            carry = 0;
        }
        result.insert(result.begin(), '0' + diff);
    }

    return BigNumber(result).trimLeadingZeros();
}

auto BigNumber::multiply(const BigNumber& other) const -> BigNumber {
    if (*this == 0 || other == 0) {
        return {"0"};
    }

    bool resultNegative = isNegative() != other.isNegative();
    BigNumber b1 = abs();
    BigNumber b2 = other.abs();

    std::vector<int> result(b1.numberString_.size() + b2.numberString_.size(),
                            0);

    for (int i = b1.numberString_.size() - 1; i >= 0; --i) {
        for (int j = b2.numberString_.size() - 1; j >= 0; --j) {
            int mul = (b1.numberString_[i] - '0') * (b2.numberString_[j] - '0');
            int sum = mul + result[i + j + 1];

            result[i + j + 1] = sum % 10;
            result[i + j] += sum / 10;
        }
    }

    std::string resultStr;
    for (int num : result) {
        if (!resultStr.empty() || num != 0) {
            resultStr.push_back(num + '0');
        }
    }

    if (resultStr.empty()) {
        resultStr = "0";
    }

    if (resultNegative && resultStr != "0") {
        resultStr.insert(resultStr.begin(), '-');
    }

    return {resultStr};
}

auto BigNumber::divide(const BigNumber& other) const -> BigNumber {
    if (other == 0) {
        throw std::invalid_argument("Division by zero");
    }

    bool resultNegative = isNegative() != other.isNegative();
    BigNumber dividend = abs();
    BigNumber divisor = other.abs();
    BigNumber quotient("0");
    BigNumber current("0");

    for (char i : dividend.numberString_) {
        current = current * 10 + BigNumber(std::string(1, i));
        int count = 0;
        while (current >= divisor) {
            current = current - divisor;
            ++count;
        }
        quotient = quotient * 10 + BigNumber(std::to_string(count));
    }

    quotient = quotient.trimLeadingZeros();
    if (resultNegative && quotient != 0) {
        quotient = quotient.negate();
    }

    return quotient;
}

auto BigNumber::pow(int exponent) const -> BigNumber {
    if (exponent < 0) {
        LOG_F(ERROR, "Powers less than 0 are not supported");
        return {"0"};
    }
    if (exponent == 0) {
        return {"1"};
    }
    if (exponent == 1) {
        return *this;
    }
    BigNumber result = std::string("1");
    BigNumber base = *this;
    while (exponent != 0) {
        if ((exponent & 1) != 0) {
            result = result.multiply(base);
        }
        exponent >>= 1;
        base = base.multiply(base);
    }
    return result;
}

auto BigNumber::trimLeadingZeros() const -> BigNumber {
    BigNumber b = *this;
    bool negative = b.isNegative();
    if (negative) {
        b.numberString_.erase(0, 1);  // Remove the negative sign temporarily
    }
    size_t nonZeroPos = b.numberString_.find_first_not_of('0');
    if (nonZeroPos != std::string::npos) {
        b.numberString_ = b.numberString_.substr(nonZeroPos);
    } else {
        b.numberString_ = "0";
    }
    if (negative && b.numberString_ != "0") {
        b.numberString_.insert(b.numberString_.begin(), '-');
    }
    return b;
}

auto operator>(const BigNumber& b1, const BigNumber& b2) -> bool {
    if (b1.isNegative() || b2.isNegative()) {
        if (b1.isNegative() && b2.isNegative()) {
            return b2.abs() > b1.abs();
        }
        return !b1.isNegative();
    }
    BigNumber b1Trimmed = b1.trimLeadingZeros();
    BigNumber b2Trimmed = b2.trimLeadingZeros();
    if (b1Trimmed.numberString_.size() != b2Trimmed.numberString_.size()) {
        return b1Trimmed.numberString_.size() > b2Trimmed.numberString_.size();
    }
    return b1Trimmed.numberString_ > b2Trimmed.numberString_;
}
}  // namespace atom::algorithm
