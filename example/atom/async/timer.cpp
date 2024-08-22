#include "atom/async/timer.hpp"

#include <iostream>
#include <thread>

void task1() { std::cout << "Task 1 executed!" << std::endl; }

void task2(int value) {
    std::cout << "Task 2 executed with value: " << value << std::endl;
}

int main() {
    // 创建一个Timer对象
    atom::async::Timer timer;

    // 设置一个延迟执行的任务（一次性任务）
    auto future1 = timer.setTimeout(task1, 2000);  // 2秒后执行task1
    future1.get();  // 获取任务的结果（等待执行完成）

    // 设置一个定时重复任务（每3秒执行一次，重复5次）
    timer.setInterval(task2, 3000, 5, 1, 42);  // 任务优先级为1，参数为42

    // 设置一个匿名函数任务（lambda表达式）
    auto future2 = timer.setTimeout(
        []() {
            std::cout << "Lambda task executed after 1 second!" << std::endl;
        },
        1000);  // 1秒后执行

    future2.get();  // 获取lambda任务的结果（等待执行完成）

    // 模拟暂停定时器
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Pausing timer..." << std::endl;
    timer.pause();

    // 暂停2秒
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 恢复定时器
    std::cout << "Resuming timer..." << std::endl;
    timer.resume();

    // 等待一段时间后取消所有任务
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "Cancelling all tasks..." << std::endl;
    timer.cancelAllTasks();

    // 停止定时器
    timer.stop();

    return 0;
}
