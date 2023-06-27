#include "threadpool.hpp"

namespace OpenAPT::Thread
{
    ThreadPool::ThreadPool(size_t num_threads)
        : stop_(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers_.emplace_back([this]()
                                  {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty())
                    {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            } });
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto &worker : workers_)
        {
            worker.join();
        }
    }
    
    void ThreadPool::wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]()
                        { return tasks_.empty(); });
    }
}
