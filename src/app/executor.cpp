#include "executor.hpp"

#include <algorithm>

AsyncExecutor::AsyncExecutor(size_t thread_count)
    : stop_flag(false), active_threads(0), task_counter(0) {
    resize(thread_count);
}

AsyncExecutor::~AsyncExecutor() {
    shutdown();
}

bool AsyncExecutor::cancel_task(size_t task_id) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    auto it = std::find_if(tasks.begin(), tasks.end(),
        [task_id](const Task& task) { return task.task_id == task_id; });

    if (it != tasks.end()) {
        it->is_cancelled = true;
        return true;
    }

    return false;
}

void AsyncExecutor::resize(size_t new_thread_count) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    size_t current_count = workers.size();
    if (new_thread_count == current_count)
        return;

    if (new_thread_count < current_count) {
        stop_flag = true;
        condition.notify_all();
        for (size_t i = 0; i < current_count - new_thread_count; ++i) {
            if (workers[i].joinable()) {
                workers[i].join();
            }
        }
        workers.resize(new_thread_count);
        stop_flag = false;
    } else {
        for (size_t i = current_count; i < new_thread_count; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop_flag || !tasks.empty();
                        });

                        if (stop_flag && tasks.empty()) {
                            return;
                        }

                        ++active_threads;

                        task = std::move(tasks.front());
                        std::pop_heap(tasks.begin(), tasks.end());
                        tasks.pop_back();
                    }
                    if (!task.is_cancelled) {
                        task();
                    }
                    --active_threads;
                }
            });
        }
    }
}

void AsyncExecutor::shutdown(bool force) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_flag = true;
    }

    if (!force) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this] { return tasks.empty(); });
    }

    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers.clear();
}

void AsyncExecutor::shutdown_delayed(std::chrono::milliseconds delay) {
    std::thread([this, delay] {
        std::this_thread::sleep_for(delay);
        shutdown();
    }).detach();
}

size_t AsyncExecutor::get_active_threads() const {
    return active_threads;
}

size_t AsyncExecutor::get_task_queue_size() const {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.size();
}

bool AsyncExecutor::Task::operator<(const Task& other) const {
    return priority < other.priority;
}

void AsyncExecutor::Task::operator()() {
    func();
}
