#include <iostream>
#include <string>
#include <coroutine>
#include "atom/io/asyncio.hpp"  // 假设 asyncio.hpp 是头文件的名称

// 定义一个简单的协程函数来演示异步文件操作
atom::io::FileWriter example_async_operations() {
    std::string filename = "example.txt";
    std::string data_to_write = "Hello, World!";
    std::string read_data;
    std::size_t read_size = 1024;

    // 异步写入文件
    co_await atom::io::async_write(filename, data_to_write);
    std::cout << "Data written to file: " << filename << std::endl;

    // 异步读取文件
    co_await atom::io::async_read(filename, read_data, read_size);
    std::cout << "Data read from file: " << read_data << std::endl;

    // 异步复制文件
    std::string copy_filename = "example_copy.txt";
    co_await atom::io::async_copy(filename, copy_filename);
    std::cout << "File copied to: " << copy_filename << std::endl;

    // 异步删除文件
    co_await atom::io::async_delete(filename);
    std::cout << "File deleted: " << filename << std::endl;

    // 异步删除复制的文件
    co_await atom::io::async_delete(copy_filename);
    std::cout << "Copied file deleted: " << copy_filename << std::endl;
}

int main() {
    // 启动协程
    example_async_operations();
    return 0;
}