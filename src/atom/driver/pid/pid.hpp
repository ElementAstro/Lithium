#pragma once

#include <memory>

class PIDImpl;

/**
 * @brief The PID class implements a Proportional-Integral-Derivative controller.
 */
class PID
{
public:
    /**
     * @brief Constructor for the PID controller.
     * @param dt The time step for the controller.
     * @param max The maximum output value of the controller.
     * @param min The minimum output value of the controller.
     * @param Kp The proportional gain.
     * @param Kd The derivative gain.
     * @param Ki The integral gain.
     */
    explicit PID(double dt, double max, double min, double Kp, double Kd, double Ki);

    /**
     * @brief Set the limits for the integrator term.
     * @param min The minimum value of the integrator term.
     * @param max The maximum value of the integrator term.
     */
    void setIntegratorLimits(double min, double max);

    /**
     * @brief Set the time constant (Tau) for the derivative component.
     * @param value The value of the time constant.
     */
    void setTau(double value);

    /**
     * @brief Calculate the control output for the given setpoint and process variable.
     * @param setpoint The desired setpoint value.
     * @param pv The process variable (current measurement).
     * @return The calculated control output.
     */
    double calculate(double setpoint, double pv);

    /**
     * @brief Get the proportional term of the controller.
     * @return The proportional term value.
     */
    double propotionalTerm() const;

    /**
     * @brief Get the integral term of the controller.
     * @return The integral term value.
     */
    double integralTerm() const;

    /**
     * @brief Get the derivative term of the controller.
     * @return The derivative term value.
     */
    double derivativeTerm() const;

private:
    std::unique_ptr<PIDImpl> pimpl; /**< Pointer to the implementation of the PID controller. */
};
