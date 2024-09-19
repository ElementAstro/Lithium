#include <iostream>
#include "atom/memory/memory.hpp"

int main() {
    // 创建一个 MemoryPool 对象
    MemoryPool<int> pool;

    // 分配内存
    int* p1 = pool.allocate(10);  // 分配 10 个 int 的内存
    int* p2 = pool.allocate(5);   // 分配 5 个 int 的内存

    // 使用分配的内存存储一些整数值
    for (int i = 0; i < 10; ++i) {
        p1[i] = i * 10;
    }
    for (int i = 0; i < 5; ++i) {
        p2[i] = i * 20;
    }

    // 打印存储的整数值
    std::cout << "p1 values: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << p1[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "p2 values: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << p2[i] << " ";
    }
    std::cout << std::endl;

    // 释放内存
    pool.deallocate(p1, 10);
    pool.deallocate(p2, 5);

    return 0;
}
