#include <iostream>

#include "atom/memory/shared.hpp"

int main() {
    try {
        // 创建一个 SharedMemory 对象
        atom::connection::SharedMemory<int> sharedMemory("MySharedMemory");

        // 写入数据到共享内存
        int dataToWrite = 42;
        sharedMemory.write(dataToWrite);
        std::cout << "Data written to shared memory: " << dataToWrite
                  << std::endl;

        // 从共享内存读取数据
        int dataRead = sharedMemory.read();
        std::cout << "Data read from shared memory: " << dataRead << std::endl;

        // 检查共享内存是否被占用
        bool occupied = sharedMemory.isOccupied();
        std::cout << "Is shared memory occupied? " << (occupied ? "Yes" : "No")
                  << std::endl;

        // 清空共享内存
        sharedMemory.clear();
        std::cout << "Shared memory cleared." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
