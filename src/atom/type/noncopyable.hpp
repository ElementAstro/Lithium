/*
 * noncopyable.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: A simple implementation of noncopyable.

**************************************************/

#ifndef ATOM_EXPERIMENT_NONCOPYABLE_HPP
#define ATOM_EXPERIMENT_NONCOPYABLE_HPP

class NonCopyable {
public:
    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;
    auto operator=(const NonCopyable &) -> NonCopyable & = delete;

    NonCopyable(NonCopyable &&) = delete;
    auto operator=(NonCopyable &&) -> NonCopyable & = delete;
};

#endif
