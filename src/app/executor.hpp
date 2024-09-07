#ifndef ASYNC_EXECUTOR_HPP
#define ASYNC_EXECUTOR_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class AsyncExecutor {
public:
    explicit AsyncExecutor(
        size_t thread_count = std::thread::hardware_concurrency());
    ~AsyncExecutor();

    template <typename F, typename... Args>
    auto submit(int priority, F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop_flag) {
                throw std::runtime_error("Submit on stopped AsyncExecutor");
            }

            tasks.push_back(
                Task{[task]() { (*task)(); }, priority, false, task_counter++});
            std::push_heap(tasks.begin(), tasks.end());
        }

        condition.notify_one();
        return result;
    }

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>> {
        return submit(0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    bool cancel_task(size_t task_id);
    void resize(size_t new_thread_count);
    void shutdown(bool force = false);
    void shutdown_delayed(std::chrono::milliseconds delay);
    size_t get_active_threads() const;
    size_t get_task_queue_size() const;

private:
    struct Task {
        std::function<void()> func;
        int priority;
        bool is_cancelled;
        size_t task_id;

        bool operator<(const Task& other) const;
        void operator()();
    };

    std::vector<std::thread> workers;
    std::vector<Task> tasks;
    mutable std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop_flag;
    std::atomic<size_t> active_threads;
    std::atomic<size_t> task_counter;
};

#endif  // ASYNC_EXECUTOR_HPP
