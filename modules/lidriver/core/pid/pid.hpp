/*
 * pid.hpp
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

#pragma once

#include <memory>

class PIDImpl;

class PID
{
public:
    // Kp - proportional gain
    // Ki - integral gain
    // Kd - derivative gain
    // dt - loop interval time
    // max - maximum value of manipulated variable
    // min - minimum value of manipulated variable
    PID(double dt, double max, double min, double Kp, double Kd, double Ki);

    void setIntegratorLimits(double min, double max);
    void setTau(double value);

    // Returns the manipulated variable given a setpoint and current process value
    double calculate(double setpoint, double pv);
    double propotionalTerm() const;
    double integralTerm() const;
    double derivativeTerm() const;

private:
    std::unique_ptr<PIDImpl> pimpl;
};

class PIDImpl
{
public:
    PIDImpl(double dt, double max, double min, double Kp, double Kd, double Ki);

    void setIntegratorLimits(double min, double max);
    void setTau(double value);

    double calculate(double setpoint, double measurement);
    double propotionalTerm() const;
    double integralTerm() const;
    double derivativeTerm() const;

private:
    // Sample Time
    double m_T{1};
    // Derivative Low-Pass filter time constant
    double m_Tau{2};

    // Output limits
    double m_Max{0};
    double m_Min{0};

    // Integrator Limits
    double m_IntegratorMin{0};
    double m_IntegratorMax{0};

    // Gains
    double m_Kp{0};
    double m_Kd{0};
    double m_Ki{0};

    // Controller volatile data
    double m_PreviousError{0};
    double m_PreviousMeasurement{0};

    // Terms
    double m_PropotionalTerm{0};
    double m_IntegralTerm{0};
    double m_DerivativeTerm{0};
};