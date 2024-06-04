#include "pid.hpp"

#include <cmath>

namespace atom::algorithm {
PID::PID(double dt, double max, double min, double Kp, double Kd, double Ki)
    : pimpl(std::make_unique<PIDImpl>(dt, max, min, Kp, Kd, Ki)) {}

void PID::setIntegratorLimits(double min, double max) {
    pimpl->setIntegratorLimits(min, max);
}

void PID::setTau(double value) { pimpl->setTau(value); }

double PID::calculate(double setpoint, double pv) {
    return pimpl->calculate(setpoint, pv);
}

double PID::propotionalTerm() const { return pimpl->propotionalTerm(); }

double PID::integralTerm() const { return pimpl->integralTerm(); }

double PID::derivativeTerm() const { return pimpl->derivativeTerm(); }

PIDImpl::PIDImpl(double dt, double max, double min, double Kp, double Kd,
                 double Ki)
    : m_T(dt), m_Max(max), m_Min(min), m_Kp(Kp), m_Kd(Kd), m_Ki(Ki) {}

PIDImpl::~PIDImpl() {}

void PIDImpl::setIntegratorLimits(double min, double max) {
    m_IntegratorMin = min;
    m_IntegratorMax = max;
}

void PIDImpl::setTau(double value) { m_Tau = value; }

double PIDImpl::calculate(double setpoint, double measurement) {
    double error = setpoint - measurement;
    m_PropotionalTerm = m_Kp * error;

    m_IntegralTerm =
        m_IntegralTerm + 0.5 * m_Ki * m_T * (error + m_PreviousError);

    if (m_IntegratorMin || m_IntegratorMax)
        m_IntegralTerm = std::min(m_IntegratorMax,
                                  std::max(m_IntegratorMin, m_IntegralTerm));

    m_DerivativeTerm = -(2.0f * m_Kd * (measurement - m_PreviousMeasurement) +
                         (2.0f * m_Tau - m_T) * m_DerivativeTerm) /
                       (2.0f * m_Tau + m_T);

    double output = m_PropotionalTerm + m_IntegralTerm + m_DerivativeTerm;
    output = std::min(m_Max, std::max(m_Min, output));

    m_PreviousError = error;
    m_PreviousMeasurement = measurement;

    return output;
}

double PIDImpl::propotionalTerm() const { return m_PropotionalTerm; }
double PIDImpl::integralTerm() const { return m_IntegralTerm; }
double PIDImpl::derivativeTerm() const { return m_DerivativeTerm; }

}  // namespace atom::algorithm
