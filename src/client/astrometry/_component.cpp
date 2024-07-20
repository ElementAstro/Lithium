/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-07-20

Description: Component wrapper for Astap

**************************************************/

#include "_component.hpp"
#include <memory>

#include "astrometry.hpp"

#include "atom/log/loguru.hpp"
#include "function/type_info.hpp"

AstapComponent::AstapComponent(const std::string& name)
    : Component(name),
      m_solver(std::make_shared<AstrometrySolver>("astrometry")) {
    LOG_F(INFO, "AstapComponent Constructed");

    def("connect", &AstrometrySolver::connect, m_solver, "main",
        "Connect to astrometry solver");
    def("disconnect", &AstrometrySolver::disconnect, m_solver, "main",
        "Disconnect from astrometry solver");
    def("reconnect", &AstrometrySolver::reconnect, m_solver, "main",
        "Reconnect to astrometry solver");
    def("isConnected", &AstrometrySolver::isConnected, m_solver, "main",
        "Check if astrometry solver is connected");
    def("scanSolver", &AstrometrySolver::scanSolver, m_solver, "main",
        "Scan for astrometry solver");
    def("solveImage", &AstrometrySolver::solveImage, m_solver, "main",
        "Solve image");
    def("getSolveResult", &AstrometrySolver::getSolveResult, m_solver, "main",
        "Get solve result");

    addVariable("astrometry.instance", m_solver, "Astap solver instance");
    this->defType<AstrometrySolver>("astrometry",
                                    atom::meta::userType<AstrometrySolver>());
}

AstapComponent::~AstapComponent() { LOG_F(INFO, "AstapComponent Destructed"); }

auto AstapComponent::initialize() -> bool {
    LOG_F(INFO, "AstapComponent Initialized");
    return true;
}

auto AstapComponent::destroy() -> bool {
    LOG_F(INFO, "AstapComponent Destroyed");
    return true;
}
