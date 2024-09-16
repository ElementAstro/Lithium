#include "executor.hpp"

#include <algorithm>

#include "atom/log/loguru.hpp"

AsyncExecutor::AsyncExecutor(size_t thread_count)
    : stop_flag(false), active_threads(0), task_counter(0) {
    LOG_F(INFO, "Initializing AsyncExecutor with {} threads", thread_count);
    resize(thread_count);
}

AsyncExecutor::~AsyncExecutor() {
    LOG_F(INFO, "Shutting down AsyncExecutor");
    shutdown();
}

bool AsyncExecutor::cancel_task(size_t task_id) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    auto it = std::find_if(
        tasks.begin(), tasks.end(),
        [task_id](const Task& task) { return task.task_id == task_id; });

    if (it != tasks.end()) {
        it->is_cancelled = true;
        LOG_F(INFO, "Task {} cancelled", task_id);
        return true;
    }

    LOG_F(WARNING, "Task {} not found for cancellation", task_id);
    return false;
}

void AsyncExecutor::resize(size_t new_thread_count) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    size_t current_count = workers.size();
    LOG_F(INFO, "Resizing thread pool from {} to {} threads", current_count,
          new_thread_count);

    if (new_thread_count == current_count)
        return;

    if (new_thread_count < current_count) {
        stop_flag = true;
        condition.notify_all();
        for (size_t i = 0; i < current_count - new_thread_count; ++i) {
            if (workers[i].joinable()) {
                workers[i].join();
                LOG_F(INFO, "Thread {} joined", i);
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
                            LOG_F(INFO, "Thread exiting due to stop flag");
                            return;
                        }

                        ++active_threads;
                        LOG_F(INFO, "Thread {} started, active threads: {}",
                              std::this_thread::get_id(), active_threads);

                        task = std::move(tasks.front());
                        std::pop_heap(tasks.begin(), tasks.end());
                        tasks.pop_back();
                    }
                    if (!task.is_cancelled) {
                        LOG_F(INFO, "Executing task {}", task.task_id);
                        task();
                    } else {
                        LOG_F(INFO, "Task {} is cancelled", task.task_id);
                    }
                    --active_threads;
                    LOG_F(INFO, "Thread {} finished task, active threads: {}",
                          std::this_thread::get_id(), active_threads);
                }
            });
            LOG_F(INFO, "Thread {} created", i);
        }
    }
}

void AsyncExecutor::shutdown(bool force) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_flag = true;
        LOG_F(INFO, "Shutdown initiated, force: {}", force);
    }

    if (!force) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this] { return tasks.empty(); });
        LOG_F(INFO, "All tasks completed, proceeding with shutdown");
    }

    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
            LOG_F(INFO, "Thread {} joined during shutdown", worker.get_id());
        }
    }
    workers.clear();
    LOG_F(INFO, "Shutdown complete");
}

void AsyncExecutor::shutdown_delayed(std::chrono::milliseconds delay) {
    LOG_F(INFO, "Shutdown delayed by {} milliseconds", delay.count());
    std::thread([this, delay] {
        std::this_thread::sleep_for(delay);
        shutdown();
    }).detach();
}

size_t AsyncExecutor::get_active_threads() const {
    LOG_F(INFO, "Getting active threads count: {}", active_threads);
    return active_threads;
}

size_t AsyncExecutor::get_task_queue_size() const {
    std::unique_lock<std::mutex> lock(queue_mutex);
    size_t size = tasks.size();
    LOG_F(INFO, "Getting task queue size: {}", size);
    return size;
}

bool AsyncExecutor::Task::operator<(const Task& other) const {
    return priority < other.priority;
}

void AsyncExecutor::Task::operator()() {
    LOG_F(INFO, "Executing task function");
    func();
}