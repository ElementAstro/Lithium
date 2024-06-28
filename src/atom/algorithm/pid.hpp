#ifndef ATOM_ALGORITHM_PID_HPP
#define ATOM_ALGORITHM_PID_HPP

#include <memory>

namespace atom::algorithm {

class PIDImpl {
public:
    PIDImpl(double dt, double max, double min, double Kp, double Kd, double Ki);
    ~PIDImpl();
    void setIntegratorLimits(double min, double max);
    void setTau(double value);
    auto calculate(double setpoint, double measurement) -> double;
    [[nodiscard]] auto propotionalTerm() const -> double;
    [[nodiscard]] auto integralTerm() const -> double;
    [[nodiscard]] auto derivativeTerm() const -> double;

private:
    double m_T{1};
    double m_Tau{2};
    double m_Max{0};
    double m_Min{0};
    double m_IntegratorMin{0};
    double m_IntegratorMax{0};
    double m_Kp{0};
    double m_Kd{0};
    double m_Ki{0};
    double m_PreviousError{0};
    double m_PreviousMeasurement{0};
    double m_PropotionalTerm{0};
    double m_IntegralTerm{0};
    double m_DerivativeTerm{0};
};

/**
 * @brief The PID class implements a Proportional-Integral-Derivative
 * controller.
 */
class PID {
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
    PID(double dt, double max, double min, double Kp, double Kd, double Ki);

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
     * @brief Calculate the control output for the given setpoint and process
     * variable.
     * @param setpoint The desired setpoint value.
     * @param pv The process variable (current measurement).
     * @return The calculated control output.
     */
    auto calculate(double setpoint, double pv) -> double;

    /**
     * @brief Get the proportional term of the controller.
     * @return The proportional term value.
     */
    [[nodiscard]] auto propotionalTerm() const -> double;

    /**
     * @brief Get the integral term of the controller.
     * @return The integral term value.
     */
    [[nodiscard]] auto integralTerm() const -> double;

    /**
     * @brief Get the derivative term of the controller.
     * @return The derivative term value.
     */
    [[nodiscard]] auto derivativeTerm() const -> double;

private:
    std::unique_ptr<PIDImpl>
        pimpl_; /**< Pointer to the implementation of the PID controller. */
};
}  // namespace atom::algorithm

#endif
