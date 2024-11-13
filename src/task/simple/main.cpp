#include "sequencer.hpp"
#include "target.hpp"
#include <iostream>
#include <memory>

using namespace lithium::sequencer;

int main() {
    // 创建一个 ExposureSequence 实例
    ExposureSequence sequence;

    // 设置回调函数
    sequence.setOnSequenceStart([]() {
        std::cout << "Sequence started." << std::endl;
    });

    sequence.setOnSequenceEnd([]() {
        std::cout << "Sequence ended." << std::endl;
    });

    sequence.setOnTargetStart([](const std::string& name, TargetStatus status) {
        std::cout << "Target " << name << " started with status " << static_cast<int>(status) << "." << std::endl;
    });

    sequence.setOnTargetEnd([](const std::string& name, TargetStatus status) {
        std::cout << "Target " << name << " ended with status " << static_cast<int>(status) << "." << std::endl;
    });

    sequence.setOnError([](const std::string& name, const std::exception& e) {
        std::cerr << "Error in target " << name << ": " << e.what() << std::endl;
    });

    // 创建并添加目标
    auto target1 = std::make_unique<Target>("Target1");
    target1->setOnStart([](const std::string& name) {
        std::cout << "Target " << name << " is starting." << std::endl;
    });
    target1->setOnEnd([](const std::string& name, TargetStatus status) {
        std::cout << "Target " << name << " has ended with status " << static_cast<int>(status) << "." << std::endl;
    });

    sequence.addTarget(std::move(target1));

    // 执行所有目标
    sequence.executeAll();

    // 等待一段时间以模拟执行过程
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 停止序列
    sequence.stop();

    return 0;
}