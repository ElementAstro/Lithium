#pragma once

#include "atom/property/task/task.hpp"

#include <mutex>

#ifdef ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class TaskContainer
{
public:
    void AddTask(const std::shared_ptr<Atom::Task::SimpleTask> &task);

    std::shared_ptr<Atom::Task::SimpleTask> GetTask(const std::string &name);

    void RemoveTask(const std::string &name);

    std::vector<std::shared_ptr<Atom::Task::SimpleTask>> GetAllTasks();

private:
    std::unordered_map<std::string, std::shared_ptr<Atom::Task::SimpleTask>> tasks;
    std::mutex mutex;
};
