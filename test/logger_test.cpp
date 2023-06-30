#include "../src/logger/log_manager.hpp"

int main()
{
    OpenAPT::Logger::LoggerManager logger;
    std::string logsFolder = ".\\log";
    logger.scanLogsFolder(logsFolder);
    logger.analyzeLogs();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
