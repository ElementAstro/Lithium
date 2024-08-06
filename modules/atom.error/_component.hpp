/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-IO

**************************************************/

#ifndef ATOM_IO_COMPONENT_HPP
#define ATOM_IO_COMPONENT_HPP

#include "atom/components/component.hpp"

namespace atom::error {
class ErrorStack;
}

class ErrorComponent : public Component {
public:
    explicit ErrorComponent(const std::string &name);
    ~ErrorComponent() override;

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    auto initialize() -> bool override;
    auto destroy() -> bool override;

private:
    std::shared_ptr<atom::error::ErrorStack> errorStack_;
};

#endif
