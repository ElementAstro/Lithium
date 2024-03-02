/*
 * notifier.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Notifier

**************************************************/

#ifndef __Notifier_HPP
#define __Notifier_HPP

#include "kevdefs.hpp"

#include <memory>

ATOM_NS_BEGIN

class Notifier {
public:
    virtual ~Notifier() {}
    virtual bool init() = 0;
    virtual bool ready() = 0;
    virtual void notify() = 0;
    virtual SOCKET_FD getReadFD() = 0;
    virtual Result onEvent(KMEvent ev) = 0;

    static std::unique_ptr<Notifier> createNotifier();
};

using NotifierPtr = std::unique_ptr<Notifier>;

ATOM_NS_END

#endif
