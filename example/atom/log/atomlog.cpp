#include "atom/log/atomlog.hpp"

void demonstrateLogger() {
    // Specify the log file name and initialize the logger
    atom::log::Logger logger("logfile.txt", atom::log::LogLevel::DEBUG);

    // Log messages with different severity levels
    logger.trace("This is a trace message with value: {}", 42);
    logger.debug("This is a debug message.");
    logger.info("This is an info message.");
    logger.warn("This is a warning message.");
    logger.error("This is an error message.");
    logger.critical("This is a critical message.");
}

int main() {
    demonstrateLogger();
    return 0;
}
