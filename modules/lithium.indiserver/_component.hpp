
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-16

Description: Simple wrapper around INDI server with atom component API
compatibility

**************************************************/

#ifndef LITHIUM_INDISERVER_COMPONENT_HPP
#define LITHIUM_INDISERVER_COMPONENT_HPP

#include "atom/components/component.hpp"

class INDIManager;  // Forward declaration

class INDIServerComponent : public Component {
public:
    explicit INDIServerComponent(const std::string& name);
    virtual ~INDIServerComponent();

    bool initialize() override;
    bool destroy() override;

private:
    std::shared_ptr<INDIManager> m_manager;
};
#endif
