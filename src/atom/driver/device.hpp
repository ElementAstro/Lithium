/*
 * device.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Defination

*************************************************/

#ifndef ATOM_DRIVER_HPP
#define ATOM_DRIVER_HPP

#include <any>
#include <functional>
#include <memory>
#include <thread>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/components/templates/shared_component.hpp"

#include "exception.hpp"

class AtomDriver : public SharedComponent {
public:
    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    explicit AtomDriver(const std::string &name);

    virtual ~AtomDriver();

    virtual bool initialize() override;

    // -------------------------------------------------------------------
    // Driver basic methods
    // -------------------------------------------------------------------

    virtual bool connect(const json &params);

    virtual bool disconnect(const json &params);

    virtual bool reconnect(const json &params);

    virtual bool isConnected();

    // -------------------------------------------------------------------
    // Driver basic methods vrapper (from command dispatcher)
    // -------------------------------------------------------------------

    json Connect(const json &params);

    json Disconnect(const json &params);

    json Reconnect(const json &params);

    json IsConnected(const json &params);

private:
    std::string m_name;
    std::string m_uuid;
};

#endif
