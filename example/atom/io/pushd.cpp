#include "atom/io/pushd.hpp"

#include <iostream>

int main() {
    // 创建一个 DirectoryStack 实例
    DirectoryStack dirStack;

    // 显示当前目录
    std::cout << "当前目录: ";
    dirStack.show_current_directory();

    // 将当前目录压入堆栈并切换到新目录
    std::filesystem::path newDir = "/path/to/new/directory";
    dirStack.pushd(newDir);
    std::cout << "切换到新目录: ";
    dirStack.show_current_directory();

    // 查看堆栈顶部的目录
    std::cout << "堆栈顶部的目录: ";
    dirStack.peek();

    // 显示当前的目录堆栈
    std::cout << "当前的目录堆栈: ";
    dirStack.dirs();

    // 从堆栈中弹出目录并切换回去
    dirStack.popd();
    std::cout << "切换回原目录: ";
    dirStack.show_current_directory();

    // 将目录堆栈保存到文件
    std::string filename = "dir_stack.txt";
    dirStack.save_stack_to_file(filename);
    std::cout << "目录堆栈已保存到文件: " << filename << std::endl;

    // 清空目录堆栈
    dirStack.clear();
    std::cout << "目录堆栈已清空" << std::endl;

    // 从文件加载目录堆栈
    dirStack.load_stack_from_file(filename);
    std::cout << "目录堆栈已从文件加载: " << filename << std::endl;

    // 显示加载后的目录堆栈
    std::cout << "加载后的目录堆栈: ";
    dirStack.dirs();

    // 获取目录堆栈的大小
    size_t stackSize = dirStack.size();
    std::cout << "目录堆栈的大小: " << stackSize << std::endl;

    // 检查目录堆栈是否为空
    bool isEmpty = dirStack.is_empty();
    std::cout << "目录堆栈是否为空: " << (isEmpty ? "是" : "否") << std::endl;

    return 0;
}
