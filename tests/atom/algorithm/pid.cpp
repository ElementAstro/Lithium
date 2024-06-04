#include "atom/algorithm/pid.hpp"
#include <gtest/gtest.h>

// Tests for PID class
TEST(PIDTest, Calculate) {
    atom::algorithm::PID pid(0.1, 100, -100, 0.1, 0.01, 0.5);
    double output = pid.calculate(1.0, 0.0);
    EXPECT_NEAR(output, 0.6,
                1e-2);
}

TEST(PIDTest, ProportionalTerm) {
    atom::algorithm::PID pid(0.1, 100, -100, 0.1, 0.01, 0.5);
    pid.calculate(1.0, 0.0);
    EXPECT_NEAR(pid.propotionalTerm(), 0.1,
                1e-2);
}

TEST(PIDTest, IntegralTerm) {
    atom::algorithm::PID pid(0.1, 100, -100, 0.1, 0.01, 0.5);
    pid.calculate(1.0, 0.0);
    EXPECT_NEAR(pid.integralTerm(), 0.05,
                1e-2);
}

TEST(PIDTest, DerivativeTerm) {
    atom::algorithm::PID pid(0.1, 100, -100, 0.1, 0.01, 0.5);
    pid.calculate(1.0, 0.0);
    EXPECT_NEAR(pid.derivativeTerm(), 0.1,
                1e-2);
}

TEST(PIDTest, IntegratorLimits) {
    atom::algorithm::PID pid(0.1, 100, -100, 0.1, 0.01, 0.5);
    pid.setIntegratorLimits(-10, 10);
    pid.calculate(1.0, 0.0);
    EXPECT_NEAR(pid.integralTerm(), 0.05,
                1e-2);
}

TEST(PIDTest, SetTau) {
    atom::algorithm::PID pid(0.1, 100, -100, 0.1, 0.01, 0.5);
    pid.setTau(0.5);
    double output = pid.calculate(1.0, 0.0);
    EXPECT_NEAR(output, 0.6,
                1e-2);
}
