
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-07-20

Description: Component wrapper for Astrometry

**************************************************/

#ifndef LITHIUM_CLIENT_ASTROMETRY_COMPONENT_HPP
#define LITHIUM_CLIENT_ASTROMETRY_COMPONENT_HPP

#include "atom/components/component.hpp"

class AstrometrySolver;

class AstrometryComponent : public Component {
public:
    explicit AstrometryComponent(const std::string& name);
    ~AstrometryComponent() override;

    auto initialize() -> bool override;
    auto destroy() -> bool override;

private:
    std::shared_ptr<AstrometrySolver> m_solver;
};
#endif
