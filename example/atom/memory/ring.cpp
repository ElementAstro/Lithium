#include <iostream>

#include "atom/memory/ring.hpp"

int main() {
    // 创建一个容量为 5 的 RingBuffer 对象
    RingBuffer<int> ring(5);

    // 向缓冲区中添加元素
    ring.push(1);
    ring.push(2);
    ring.push(3);
    ring.push(4);
    ring.push(5);

    // 尝试添加第 6 个元素，应该返回 false 因为缓冲区已满
    if (!ring.push(6)) {
        std::cout << "Buffer is full, cannot push 6" << std::endl;
    }

    // 打印缓冲区中的元素
    std::cout << "Buffer contents: ";
    for (const auto& item : ring.view()) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    // 从缓冲区中弹出元素
    auto item = ring.pop();
    if (item) {
        std::cout << "Popped item: " << *item << std::endl;
    }

    // 使用 pushOverwrite 方法添加元素，覆盖最旧的元素
    ring.pushOverwrite(6);

    // 打印缓冲区中的元素
    std::cout << "Buffer contents after pushOverwrite: ";
    for (const auto& item : ring.view()) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    // 检查缓冲区是否包含某个元素
    if (ring.contains(3)) {
        std::cout << "Buffer contains 3" << std::endl;
    } else {
        std::cout << "Buffer does not contain 3" << std::endl;
    }

    // 清空缓冲区
    ring.clear();
    std::cout << "Buffer cleared. Is empty: " << (ring.empty() ? "Yes" : "No")
              << std::endl;

    return 0;
}
