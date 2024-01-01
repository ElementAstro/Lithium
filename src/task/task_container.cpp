#include "task_container.hpp"

void TaskContainer::AddTask(const std::shared_ptr<Atom::Task::SimpleTask> &task)
{
    std::lock_guard<std::mutex> lock(mutex);
    tasks[task->getName()] = task;
}

std::shared_ptr<Atom::Task::SimpleTask> TaskContainer::GetTask(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = tasks.find(name);
    if (it != tasks.end())
    {
        return it->second;
    }
    return nullptr;
}

void TaskContainer::RemoveTask(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = tasks.find(name);
    if (it != tasks.end())
    {
        tasks.erase(it);
    }
}

std::vector<std::shared_ptr<Atom::Task::SimpleTask>> TaskContainer::GetAllTasks()
{
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<std::shared_ptr<Atom::Task::SimpleTask>> result;
    for (const auto &pair : tasks)
    {
        result.push_back(pair.second);
    }
    return result;
}
