/*
 * device.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Defintion

*************************************************/

#ifndef ATOM_DRIVER_HPP
#define ATOM_DRIVER_HPP

#include <string>
#include <vector>

class AtomDriver {
public:
    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    explicit AtomDriver(std::string name);

    virtual ~AtomDriver();

    virtual auto initialize() -> bool = 0;

    virtual auto destroy() -> bool = 0;

    [[nodiscard]] auto getUUID() const -> std::string;

    [[nodiscard]] auto getName() const -> std::string;

    void setName(const std::string& newName);

    [[nodiscard]] auto getType() const -> std::string;

    // -------------------------------------------------------------------
    // Driver basic methods
    // -------------------------------------------------------------------

    virtual auto connect(const std::string& name, int timeout,
                         int maxRetry) -> bool = 0;

    virtual auto disconnect(bool force, int timeout, int maxRetry) -> bool = 0;

    virtual auto reconnect(int timeout, int maxRetry) -> bool = 0;

    virtual auto scan() -> std::vector<std::string> = 0;

    virtual auto isConnected() -> bool = 0;

private:
    std::string name_;
    std::string uuid_;
    std::string type_;
};

#endif
