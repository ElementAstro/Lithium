#include "cotask.hpp"
#include <thread>

#include "atom/log/loguru.hpp"

auto TaskScheduler::Task::promise_type::get_return_object() -> Task {
    return Task{handle_type::from_promise(*this)};
}

auto TaskScheduler::Task::promise_type::initial_suspend()
    -> std::suspend_never {
    return {};
}

auto TaskScheduler::Task::promise_type::final_suspend() noexcept
    -> std::suspend_always {
    return {};
}

void TaskScheduler::Task::promise_type::return_value(std::string value) {
    result = std::move(value);
}

auto TaskScheduler::Task::promise_type::yield_value(std::string value)
    -> std::suspend_always {
    result = std::move(value);
    return {};
}

void TaskScheduler::Task::promise_type::unhandled_exception() {
    result = std::current_exception();
    if (exceptionHandler) {
        try {
            std::rethrow_exception(std::get<std::exception_ptr>(result));
        } catch (const std::exception& e) {
            exceptionHandler(e);
        }
    }
}

// Task constructors, destructor, and move operations

TaskScheduler::Task::Task(handle_type h) : handle(h) {}

TaskScheduler::Task::~Task() {
    if (handle)
        handle.destroy();
}

TaskScheduler::Task::Task(Task&& other) noexcept : handle(other.handle) {
    other.handle = nullptr;
}

auto TaskScheduler::Task::operator=(Task&& other) noexcept -> Task& {
    if (this != &other) {
        if (handle) {
            handle.destroy();
        }
        handle = other.handle;
        other.handle = nullptr;
    }
    return *this;
}

// Task::await methods

auto TaskScheduler::Task::await_ready() const noexcept -> bool {
    return handle.done();
}

void TaskScheduler::Task::await_suspend(std::coroutine_handle<> h) const {
    handle.resume();
}

void TaskScheduler::Task::await_resume() const {}

// TaskScheduler methods

void TaskScheduler::schedule(std::string id, std::shared_ptr<Task> task) {
    tasks_[id] = std::move(task);
    LOG_F(INFO, "Scheduling task: {}", id);
}

void TaskScheduler::setGlobalExceptionHandler(
    std::function<void(const std::exception&)> handler) {
    global_exception_handler_ = std::move(handler);
}

void TaskScheduler::run() {
    while (!tasks_.empty()) {
        for (auto it = tasks_.begin(); it != tasks_.end();) {
            if (areDependenciesMet(it->second->dependencies)) {
                try {
                    if (it->second->handle) {
                        it->second->handle.resume();
                        if (it->second->handle.done()) {
                            completed_tasks_.insert(it->first);
                            it = tasks_.erase(it);
                            continue;
                        }
                    }
                } catch (const std::exception& e) {
                    handleException(e, it->second);
                    it = tasks_.erase(it);
                    continue;
                }
            }
            ++it;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

auto TaskScheduler::getResult(const Task& task) -> std::optional<std::string> {
    if (auto* result =
            std::get_if<std::string>(&task.handle.promise().result)) {
        return *result;
    }
    if (auto* exPtr =
            std::get_if<std::exception_ptr>(&task.handle.promise().result)) {
        std::rethrow_exception(*exPtr);
    }
    return std::nullopt;
}

auto TaskScheduler::areDependenciesMet(
    const std::vector<std::string>& dependencies) -> bool {
    return std::all_of(dependencies.begin(), dependencies.end(),
                       [this](const std::string& dep) {
                           return completed_tasks_.contains(dep);
                       });
}

void TaskScheduler::handleException(const std::exception& e,
                                    const std::shared_ptr<Task>& task) {
    if (task->handle.promise().exceptionHandler) {
        task->handle.promise().exceptionHandler(e);
    } else if (global_exception_handler_) {
        global_exception_handler_(e);
    } else {
        LOG_F(ERROR, "Unhandled task exception: {}", e.what());
    }
}
