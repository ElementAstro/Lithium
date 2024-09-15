#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

class TaskScheduler {
public:
    struct Task {
        struct promise_type;
        using handle_type = std::coroutine_handle<promise_type>;

        struct promise_type {
            std::variant<std::monostate, std::string, std::exception_ptr>
                result;
            std::function<void(const std::exception&)> exceptionHandler;

            auto get_return_object() -> Task;
            auto initial_suspend() -> std::suspend_never;
            auto final_suspend() noexcept -> std::suspend_always;
            void return_value(std::string value);
            auto yield_value(std::string value) -> std::suspend_always;
            void unhandled_exception();
        };

        handle_type handle;
        std::vector<std::string> dependencies;

        Task(handle_type h);
        ~Task();
        Task(const Task&) = delete;
        Task(Task&& other) noexcept;
        auto operator=(Task&& other) noexcept -> Task&;

        void set_exception_handler(
            std::function<void(const std::exception&)> handler);

        auto await_ready() const noexcept -> bool;
        void await_suspend(std::coroutine_handle<> h) const;
        void await_resume() const;
    };

    void schedule(std::string id, std::shared_ptr<Task> task);

    void setGlobalExceptionHandler(
        std::function<void(const std::exception&)> handler);

    void run();

    static auto getResult(const Task& task) -> std::optional<std::string>;

private:
    std::unordered_map<std::string, std::shared_ptr<Task>> tasks_;
    std::unordered_set<std::string> completed_tasks_;
    std::function<void(const std::exception&)> global_exception_handler_;

    auto areDependenciesMet(const std::vector<std::string>& dependencies)
        -> bool;
    void handleException(const std::exception& e,
                         const std::shared_ptr<Task>& task);
};

#endif  // TASK_SCHEDULER_H
