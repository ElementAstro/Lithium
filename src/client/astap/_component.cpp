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

#include "astap.hpp"

#include "atom/log/loguru.hpp"
#include "function/type_info.hpp"

AstapComponent::AstapComponent(const std::string& name)
    : Component(name), m_solver(std::make_shared<AstapSolver>("astap")) {
    LOG_F(INFO, "AstapComponent Constructed");

    def("connect", &AstapSolver::connect, m_solver, "main",
        "Connect to astap solver");
    def("disconnect", &AstapSolver::disconnect, m_solver, "main",
        "Disconnect from astap solver");
    def("reconnect", &AstapSolver::reconnect, m_solver, "main",
        "Reconnect to astap solver");
    def("isConnected", &AstapSolver::isConnected, m_solver, "main",
        "Check if astap solver is connected");
    def("scanSolver", &AstapSolver::scanSolver, m_solver, "main",
        "Scan for astap solver");
    def("solveImage", &AstapSolver::solveImage, m_solver, "main",
        "Solve image");
    def("getSolveResult", &AstapSolver::getSolveResult, m_solver, "main",
        "Get solve result");

    addVariable("astap.instance", m_solver, "Astap solver instance");
    this->defType<AstapSolver>("astap", atom::meta::userType<AstapSolver>());
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
