#ifndef LITHIUM_TASK_INTERFACE_TASK_HPP
#define LITHIUM_TASK_INTERFACE_TASK_HPP

#include "task/custom/cotask.hpp"

class ITask {
public:
    virtual auto run() -> TaskScheduler::Task = 0;
};

#endif
