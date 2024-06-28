#include "pid.hpp"

#include <cmath>

namespace atom::algorithm {
PID::PID(double dt, double max, double min, double Kp, double Kd, double Ki)
    : pimpl_(std::make_unique<PIDImpl>(dt, max, min, Kp, Kd, Ki)) {}

void PID::setIntegratorLimits(double min, double max) {
    pimpl_->setIntegratorLimits(min, max);
}

void PID::setTau(double value) { pimpl_->setTau(value); }

double PID::calculate(double setpoint, double pv) {
    return pimpl_->calculate(setpoint, pv);
}

double PID::propotionalTerm() const { return pimpl_->propotionalTerm(); }

double PID::integralTerm() const { return pimpl_->integralTerm(); }

double PID::derivativeTerm() const { return pimpl_->derivativeTerm(); }

PIDImpl::PIDImpl(double dt, double max, double min, double Kp, double Kd,
                 double Ki)
    : m_T(dt), m_Max(max), m_Min(min), m_Kp(Kp), m_Kd(Kd), m_Ki(Ki) {}

PIDImpl::~PIDImpl() {}

void PIDImpl::setIntegratorLimits(double min, double max) {
    m_IntegratorMin = min;
    m_IntegratorMax = max;
}

void PIDImpl::setTau(double value) { m_Tau = value; }

auto PIDImpl::calculate(double setpoint, double measurement) -> double {
    double error = setpoint - measurement;
    m_PropotionalTerm = m_Kp * error;

    m_IntegralTerm =
        m_IntegralTerm + 0.5 * m_Ki * m_T * (error + m_PreviousError);

    if ((m_IntegratorMin != 0.0) || (m_IntegratorMax != 0.0)) {
        m_IntegralTerm = std::min(m_IntegratorMax,
                                  std::max(m_IntegratorMin, m_IntegralTerm));
    }

    m_DerivativeTerm = -(2.0F * m_Kd * (measurement - m_PreviousMeasurement) +
                         (2.0F * m_Tau - m_T) * m_DerivativeTerm) /
                       (2.0F * m_Tau + m_T);

    double output = m_PropotionalTerm + m_IntegralTerm + m_DerivativeTerm;
    output = std::min(m_Max, std::max(m_Min, output));

    m_PreviousError = error;
    m_PreviousMeasurement = measurement;

    return output;
}

auto PIDImpl::propotionalTerm() const -> double { return m_PropotionalTerm; }
auto PIDImpl::integralTerm() const -> double { return m_IntegralTerm; }
auto PIDImpl::derivativeTerm() const -> double { return m_DerivativeTerm; }

}  // namespace atom::algorithm
