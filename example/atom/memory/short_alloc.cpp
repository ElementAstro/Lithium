#include "atom/memory/short_alloc.hpp"

#include <iostream>
#include <vector>


int main() {
    // 创建一个 Arena 对象，大小为 1024 字节
    atom::memory::Arena<1024> arena;

    // 创建一个 ShortAlloc 对象，使用上面的 Arena
    atom::memory::ShortAlloc<int, 1024> allocator(arena);

    // 使用 ShortAlloc 创建一个 vector
    std::vector<int, atom::memory::ShortAlloc<int, 1024>> vec(allocator);

    // 向 vector 中添加元素
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }

    // 打印 vector 中的元素
    std::cout << "Vector contents: ";
    for (const auto& item : vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    // 使用 allocateUnique 分配一个 int
    auto uniqueInt = atom::memory::allocateUnique(allocator, 42);
    std::cout << "Unique int: " << *uniqueInt << std::endl;

    return 0;
}