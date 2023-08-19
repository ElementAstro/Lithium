/*
 * pid.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-9

Description: PID (pid)

**************************************************/

#include "pid.hpp"

PID::PID(double dt, double max, double min, double Kp, double Kd, double Ki) : pimpl(std::make_unique<PIDImpl>(dt, max, min, Kp, Kd, Ki)) {}

void PID::setIntegratorLimits(double min, double max)
{
    pimpl->setIntegratorLimits(min, max);
}

void PID::setTau(double value)
{
    pimpl->setTau(value);
}

double PID::calculate(double setpoint, double pv)
{
    return pimpl->calculate(setpoint, pv);
}

double PID::propotionalTerm() const
{
    return pimpl->propotionalTerm();
}

double PID::integralTerm() const
{
    return pimpl->integralTerm();
}

double PID::derivativeTerm() const
{
    return pimpl->derivativeTerm();
}

PIDImpl::PIDImpl(double dt, double max, double min, double Kp, double Kd, double Ki) : m_T(dt),
                                                                                       m_Max(max),
                                                                                       m_Min(min),
                                                                                       m_Kp(Kp),
                                                                                       m_Kd(Kd),
                                                                                       m_Ki(Ki) {}

void PIDImpl::setIntegratorLimits(double min, double max)
{
    m_IntegratorMin = min;
    m_IntegratorMax = max;
}

void PIDImpl::setTau(double value)
{
    m_Tau = value;
}

double PIDImpl::calculate(double setpoint, double measurement)
{
    // Calculate error
    double error = setpoint - measurement;

    // Proportional term
    m_PropotionalTerm = m_Kp * error;

    // Integral term
    m_IntegralTerm += 0.5 * m_Ki * m_T * (error + m_PreviousError);

    // Clamp Integral
    if (m_IntegratorMin || m_IntegratorMax)
    {
        m_IntegralTerm = std::clamp(m_IntegralTerm, m_IntegratorMin, m_IntegratorMax);
    }

    // Derivative term (N.B. on measurement NOT error)
    m_DerivativeTerm = -(2.0 * m_Kd * (measurement - m_PreviousMeasurement) + (2.0 * m_Tau - m_T) * m_DerivativeTerm) / (2.0 * m_Tau + m_T);

    // Calculate total output
    double output = m_PropotionalTerm + m_IntegralTerm + m_DerivativeTerm;

    // Clamp Output
    output = std::clamp(output, m_Min, m_Max);

    // Save error to previous error
    m_PreviousError = error;
    m_PreviousMeasurement = measurement;

    return output;
}

double PIDImpl::propotionalTerm() const
{
    return m_PropotionalTerm;
}

double PIDImpl::integralTerm() const
{
    return m_IntegralTerm;
}

double PIDImpl::derivativeTerm() const
{
    return m_DerivativeTerm;
}