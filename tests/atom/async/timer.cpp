#include "atom/async/timer.hpp"
#include <iostream>

// 用法示例
int main()
{
    Timer timer;

    // 可取消的定时器任务
    std::future<void> future = timer.setTimeout([]()
                                                { std::cout << "Timeout!" << std::endl; },
                                                1000);
    timer.setTimeout([]()
                     { std::cout << "This should not be printed!" << std::endl; },
                     2000);
    timer.cancelAllTasks();

    // 任务函数支持返回值
    std::future<int> result = timer.setTimeout([]()
                                               { return 42; },
                                               2000);
    std::cout << "Result: " << result.get() << std::endl;

    // 定时器线程池
    for (int i = 0; i < 5; ++i)
    {
        timer.setInterval([i]()
                          { std::cout << "Interval from thread " << i << "!" << std::endl; },
                          500, 10 - i, i);
    }

    // 暂停定时器任务
    timer.pause();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 恢复定时器任务
    timer.resume();

    timer.setTimeout([]() -> void
                     { std::cout << "High priority task!" << std::endl; },
                     1000);
    timer.setTimeout([]() -> void
                     { std::cout << "Low priority task!" << std::endl; },
                     1000);

    // 添加回调函数
    timer.setCallback([]()
                      { std::cout << "Task completed!" << std::endl; });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    timer.stop();

    return 0;
}
