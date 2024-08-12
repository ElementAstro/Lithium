
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-07-20

Description: Component wrapper for Astap

**************************************************/

#ifndef LITHIUM_CLIENT_ASTAP_COMPONENT_HPP
#define LITHIUM_CLIENT_ASTAP_COMPONENT_HPP

#include "atom/components/component.hpp"

class AstapSolver;

class AstapComponent : public Component {
public:
    explicit AstapComponent(const std::string& name);
     ~AstapComponent() override;

    auto initialize() -> bool override;
    auto destroy() -> bool override;

private:
    std::shared_ptr<AstapSolver> m_solver;
};
#endif
