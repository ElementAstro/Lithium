
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-18

Description: Lithium Web Interface based on oatpp

**************************************************/

#ifndef LITHIUM_WEBSERVER_COMPONENT_HPP
#define LITHIUM_WEBSERVER_COMPONENT_HPP

#include "atom/components/component.hpp"

class ServerComponent : public Component {
public:
    explicit ServerComponent(const std::string& name);
     ~ServerComponent() override;

    bool initialize() override;
    bool destroy() override;
};
#endif
