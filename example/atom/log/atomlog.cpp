#include "atom/log/atomlog.hpp"
#include <iostream>

int main() {
    // 创建一个 Logger 实例
    atom::log::Logger logger("logfile.log", atom::log::LogLevel::DEBUG);

    // 设置日志级别
    logger.setLevel(atom::log::LogLevel::INFO);

    // 设置日志模式
    logger.setPattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    // 设置线程名称
    logger.setThreadName("MainThread");

    // 记录不同级别的日志
    logger.trace("This is a trace message: {}", 1);
    logger.debug("This is a debug message: {}", 2);
    logger.info("This is an info message: {}", 3);
    logger.warn("This is a warning message: {}", 4);
    logger.error("This is an error message: {}", 5);
    logger.critical("This is a critical message: {}", 6);

    // 启用系统日志记录
    logger.enableSystemLogging(true);

    // 注册一个新的日志接收器
    auto another_logger =
        std::make_shared<atom::log::Logger>("another_logfile.log");
    logger.registerSink(another_logger);

    // 移除日志接收器
    logger.removeSink(another_logger);

    // 清除所有日志接收器
    logger.clearSinks();

    return 0;
}