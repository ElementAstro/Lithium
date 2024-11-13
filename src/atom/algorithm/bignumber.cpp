#include "bignumber.hpp"

#include <stdexcept>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::algorithm {
auto BigNumber::add(const BigNumber& other) const -> BigNumber {
    try {
        LOG_F(INFO, "Adding {} and {}", this->numberString_,
              other.numberString_);
        if (isNegative() && other.isNegative()) {
            LOG_F(INFO, "Both numbers are negative. Negating and adding.");
            return negate().add(other.negate()).negate();
        }
        if (isNegative()) {
            LOG_F(INFO, "First number is negative. Performing subtraction.");
            return other.subtract(abs());
        }
        if (other.isNegative()) {
            LOG_F(INFO, "Second number is negative. Performing subtraction.");
            return subtract(other.abs());
        }

        std::string result;
        int carry = 0;
        int i = static_cast<int>(numberString_.length()) - 1;
        int j = static_cast<int>(other.numberString_.length()) - 1;

        while (i >= 0 || j >= 0 || carry != 0) {
            int digit1 = (i >= 0) ? numberString_[i--] - '0' : 0;
            int digit2 = (j >= 0) ? other.numberString_[j--] - '0' : 0;
            int sum = digit1 + digit2 + carry;
            result.insert(result.begin(), '0' + (sum % 10));
            carry = sum / 10;
        }

        LOG_F(INFO, "Result of addition: {}", result);
        return BigNumber(result).trimLeadingZeros();
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in BigNumber::add: {}", e.what());
        throw;
    }
}

auto BigNumber::subtract(const BigNumber& other) const -> BigNumber {
    try {
        LOG_F(INFO, "Subtracting {} from {}", other.numberString_,
              this->numberString_);
        if (isNegative() && other.isNegative()) {
            LOG_F(INFO, "Both numbers are negative. Adjusting subtraction.");
            return other.negate().subtract(negate());
        }
        if (isNegative()) {
            LOG_F(
                INFO,
                "First number is negative. Performing addition with negation.");
            return negate().add(other).negate();
        }
        if (other.isNegative()) {
            LOG_F(INFO, "Second number is negative. Performing addition.");
            return add(other.negate());
        }
        if (*this < other) {
            LOG_F(INFO, "Result will be negative.");
            return other.subtract(*this).negate();
        }

        std::string result;
        int carry = 0;
        int i = static_cast<int>(numberString_.length()) - 1;
        int j = static_cast<int>(other.numberString_.length()) - 1;

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

        LOG_F(INFO, "Result of subtraction before trimming: {}", result);
        return BigNumber(result).trimLeadingZeros();
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in BigNumber::subtract: {}", e.what());
        throw;
    }
}

auto BigNumber::multiply(const BigNumber& other) const -> BigNumber {
    try {
        LOG_F(INFO, "Multiplying {} and {}", this->numberString_,
              other.numberString_);
        if (*this == BigNumber("0") || other == BigNumber("0")) {
            LOG_F(INFO, "One of the numbers is zero. Result is 0.");
            return BigNumber("0");
        }

        bool resultNegative = isNegative() != other.isNegative();
        BigNumber b1 = abs();
        BigNumber b2 = other.abs();

        std::vector<int> result(
            b1.numberString_.size() + b2.numberString_.size(), 0);

        for (int i = static_cast<int>(b1.numberString_.size()) - 1; i >= 0;
             --i) {
            for (int j = static_cast<int>(b2.numberString_.size()) - 1; j >= 0;
                 --j) {
                int mul =
                    (b1.numberString_[i] - '0') * (b2.numberString_[j] - '0');
                int sum = mul + result[i + j + 1];

                result[i + j + 1] = sum % 10;
                result[i + j] += sum / 10;
            }
        }

        std::string resultStr;
        bool started = false;
        for (int num : result) {
            if (!started && num == 0)
                continue;
            started = true;
            resultStr.push_back(num + '0');
        }

        if (resultStr.empty()) {
            resultStr = "0";
        }

        if (resultNegative && resultStr != "0") {
            resultStr.insert(resultStr.begin(), '-');
        }

        LOG_F(INFO, "Result of multiplication: {}", resultStr);
        return {resultStr};
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in BigNumber::multiply: {}", e.what());
        throw;
    }
}

auto BigNumber::divide(const BigNumber& other) const -> BigNumber {
    try {
        LOG_F(INFO, "Dividing {} by {}", this->numberString_,
              other.numberString_);
        if (other == BigNumber("0")) {
            LOG_F(ERROR, "Division by zero");
            THROW_INVALID_ARGUMENT("Division by zero");
        }

        bool resultNegative = isNegative() != other.isNegative();
        BigNumber dividend = abs();
        BigNumber divisor = other.abs();
        BigNumber quotient("0");
        BigNumber current("0");

        for (char digit : dividend.numberString_) {
            current = current.multiply(BigNumber("10"))
                          .add(BigNumber(std::string(1, digit)));
            int count = 0;
            while (current >= divisor) {
                current = current.subtract(divisor);
                ++count;
            }
            quotient = quotient.multiply(BigNumber("10"))
                           .add(BigNumber(std::to_string(count)));
        }

        quotient = quotient.trimLeadingZeros();
        if (resultNegative && quotient != BigNumber("0")) {
            quotient = quotient.negate();
        }

        LOG_F(INFO, "Result of division: {}", quotient.numberString_);
        return quotient;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in BigNumber::divide: {}", e.what());
        throw;
    }
}

auto BigNumber::pow(int exponent) const -> BigNumber {
    try {
        LOG_F(INFO, "Raising {} to the power of {}", this->numberString_,
              exponent);
        if (exponent < 0) {
            LOG_F(ERROR, "Negative exponents are not supported");
            THROW_INVALID_ARGUMENT("Negative exponents are not supported");
        }
        if (exponent == 0) {
            return BigNumber("1");
        }
        if (exponent == 1) {
            return *this;
        }
        BigNumber result("1");
        BigNumber base = *this;
        while (exponent != 0) {
            if (exponent & 1) {
                result = result.multiply(base);
            }
            exponent >>= 1;
            if (exponent != 0) {
                base = base.multiply(base);
            }
        }
        LOG_F(INFO, "Result of exponentiation: {}", result.numberString_);
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in BigNumber::pow: {}", e.what());
        throw;
    }
}

auto BigNumber::trimLeadingZeros() const -> BigNumber {
    try {
        LOG_F(INFO, "Trimming leading zeros from {}", this->numberString_);
        std::string trimmed = numberString_;
        bool negative = false;
        size_t start = 0;

        if (!trimmed.empty() && trimmed[0] == '-') {
            negative = true;
            start = 1;
        }

        // Find the position of the first non-zero character
        size_t nonZeroPos = trimmed.find_first_not_of('0', start);
        if (nonZeroPos == std::string::npos) {
            // The number is zero
            return BigNumber("0");
        }

        trimmed = trimmed.substr(nonZeroPos);
        if (negative) {
            trimmed.insert(trimmed.begin(), '-');
        }

        LOG_F(INFO, "Trimmed number: {}", trimmed);
        return {trimmed};
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in BigNumber::trimLeadingZeros: {}", e.what());
        throw;
    }
}

auto operator>(const BigNumber& b1, const BigNumber& b2) -> bool {
    try {
        LOG_F(INFO, "Comparing if {} > {}", b1.numberString_, b2.numberString_);
        if (b1.isNegative() || b2.isNegative()) {
            if (b1.isNegative() && b2.isNegative()) {
                LOG_F(INFO, "Both numbers are negative. Flipping comparison.");
                return atom::algorithm::BigNumber(b2).abs() >
                       atom::algorithm::BigNumber(b1).abs();
            }
            return b1.isNegative() < b2.isNegative();
        }

        BigNumber b1Trimmed = b1.trimLeadingZeros();
        BigNumber b2Trimmed = b2.trimLeadingZeros();

        if (b1Trimmed.numberString_.size() != b2Trimmed.numberString_.size()) {
            return b1Trimmed.numberString_.size() >
                   b2Trimmed.numberString_.size();
        }
        return b1Trimmed.numberString_ > b2Trimmed.numberString_;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in operator>: {}", e.what());
        throw;
    }
}

void BigNumber::validate() const {
    if (numberString_.empty()) {
        THROW_INVALID_ARGUMENT("Empty string is not a valid number");
    }
    size_t start = 0;
    if (numberString_[0] == '-') {
        if (numberString_.size() == 1) {
            THROW_INVALID_ARGUMENT("Invalid number format");
        }
        start = 1;
    }
    for (size_t i = start; i < numberString_.size(); ++i) {
        if (std::isdigit(numberString_[i]) == 0) {
            THROW_INVALID_ARGUMENT("Invalid character in number string");
        }
    }
}

}  // namespace atom::algorithm