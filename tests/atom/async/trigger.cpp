#include "atom/async/trigger.hpp"

#include <iostream>

// 测试回调函数
void callback1(int value)
{
    std::cout << "Callback 1: " << value << std::endl;
}

void callback2(int value)
{
    std::cout << "Callback 2: " << value << std::endl;
}

void callback3(int value)
{
    std::cout << "Callback 3: " << value << std::endl;
}

int main()
{
    Trigger<int> trigger;

    // 注册回调函数
    trigger.registerCallback("event1", callback1, Trigger<int>::CallbackPriority::Normal);
    trigger.registerCallback("event1", callback2, Trigger<int>::CallbackPriority::High);
    trigger.registerCallback("event1", callback3, Trigger<int>::CallbackPriority::Low);

    // 触发事件
    trigger.trigger("event1", 42);

    // 取消回调函数
    trigger.unregisterCallback("event1", callback2);

    // 再次触发事件
    trigger.trigger("event1", 42);

    // 延迟触发事件
    trigger.scheduleTrigger("event1", 42, std::chrono::milliseconds(1000));

    // 异步触发事件
    std::future<void> asyncTrigger = trigger.scheduleAsyncTrigger("event1", 42);

    // 等待异步触发完成
    asyncTrigger.wait();

    // 取消触发事件
    trigger.cancelTrigger("event1");

    // 再次触发事件（被取消了，所以不会触发回调函数）

    return 0;
}
