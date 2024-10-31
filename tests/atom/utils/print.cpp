#include <gtest/gtest.h>
#include <sstream>

#include "atom/utils/print.hpp"

namespace atom::utils::test {

TEST(PrintUtilsTest, LogFunction) {
    std::ostringstream stream;
    log(stream, LogLevel::INFO, "Test message: {}", 42);
    std::string output = stream.str();
    ASSERT_TRUE(output.find("INFO") != std::string::npos);
    ASSERT_TRUE(output.find("Test message: 42") != std::string::npos);
}

TEST(PrintUtilsTest, PrintToStreamFunction) {
    std::ostringstream stream;
    printToStream(stream, "Hello, {}!", "world");
    ASSERT_EQ(stream.str(), "Hello, world!");
}

TEST(PrintUtilsTest, PrintFunction) {
    std::ostringstream stream;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(stream.rdbuf());
    print("Hello, {}!", "world");
    std::cout.rdbuf(oldCoutBuf);
    ASSERT_EQ(stream.str(), "Hello, world!");
}

TEST(PrintUtilsTest, PrintlnFunction) {
    std::ostringstream stream;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(stream.rdbuf());
    println("Hello, {}!", "world");
    std::cout.rdbuf(oldCoutBuf);
    ASSERT_EQ(stream.str(), "Hello, world!\n");
}

TEST(PrintUtilsTest, PrintToFileFunction) {
    std::string fileName = "test_output.txt";
    printToFile(fileName, "File content: {}", 123);
    std::ifstream file(fileName);
    std::string content;
    std::getline(file, content);
    ASSERT_EQ(content, "File content: 123");
    file.close();
    std::remove(fileName.c_str());
}

TEST(PrintUtilsTest, PrintColoredFunction) {
    std::ostringstream stream;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(stream.rdbuf());
    printColored(Color::RED, "Colored message: {}", 99);
    std::cout.rdbuf(oldCoutBuf);
    ASSERT_EQ(stream.str(), "\033[31mColored message: 99\033[0m");
}

TEST(PrintUtilsTest, TimerClass) {
    Timer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    double elapsed = timer.elapsed();
    ASSERT_GE(elapsed, 0.1);
}

TEST(PrintUtilsTest, CodeBlockClass) {
    CodeBlock codeBlock;
    std::ostringstream stream;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(stream.rdbuf());
    codeBlock.print("Indented message");
    std::cout.rdbuf(oldCoutBuf);
    ASSERT_EQ(stream.str(), "Indented message");
}

TEST(PrintUtilsTest, PrintStyledFunction) {
    std::ostringstream stream;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(stream.rdbuf());
    printStyled(TextStyle::BOLD, "Styled message: {}", 77);
    std::cout.rdbuf(oldCoutBuf);
    ASSERT_EQ(stream.str(), "\033[1mStyled message: 77\033[0m");
}

TEST(PrintUtilsTest, MathStatsClass) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_DOUBLE_EQ(MathStats::mean(data), 3.0);
    ASSERT_DOUBLE_EQ(MathStats::median(data), 3.0);
    ASSERT_DOUBLE_EQ(MathStats::standardDeviation(data), std::sqrt(2.0));
}

TEST(PrintUtilsTest, MemoryTrackerClass) {
    MemoryTracker tracker;
    tracker.allocate("test1", 100);
    tracker.allocate("test2", 200);
    std::ostringstream stream;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(stream.rdbuf());
    tracker.printUsage();
    std::cout.rdbuf(oldCoutBuf);
    std::string output = stream.str();
    ASSERT_TRUE(output.find("test1: 100 bytes") != std::string::npos);
    ASSERT_TRUE(output.find("test2: 200 bytes") != std::string::npos);
    ASSERT_TRUE(output.find("Total memory usage: 300 bytes") != std::string::npos);
}

TEST(PrintUtilsTest, FormatLiteralClass) {
    auto formatted = "Hello, {}!"_fmt("world");
    ASSERT_EQ(formatted, "Hello, world!");
}

}  // namespace atom::utils::test
