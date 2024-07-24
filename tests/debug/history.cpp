#include "debug/history.hpp"

#include <gtest/gtest.h>
#include <thread>

using namespace lithium::debug;

class CommandHistoryTest : public ::testing::Test {
protected:
    void SetUp() override { cmdHistory = new CommandHistory(5, "test_user"); }

    void TearDown() override { delete cmdHistory; }

    CommandHistory* cmdHistory;
};

TEST_F(CommandHistoryTest, AddCommand) {
    cmdHistory->addCommand("ls -l");
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printHistory();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("ls -l") != std::string::npos);
}

TEST_F(CommandHistoryTest, UndoCommand) {
    cmdHistory->clearHistory();
    cmdHistory->addCommand("ls -l");
    cmdHistory->addCommand("cd /home/user");
    cmdHistory->undo();
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printHistory();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("ls -l") != std::string::npos);
    EXPECT_TRUE(output.str().find("cd /home/user") == std::string::npos);
}

TEST_F(CommandHistoryTest, RedoCommand) {
    cmdHistory->clearHistory();
    cmdHistory->addCommand("ls -l");
    cmdHistory->addCommand("cd /home/user");
    cmdHistory->undo();
    cmdHistory->redo();
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printHistory();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("cd /home/user") != std::string::npos);
}

TEST_F(CommandHistoryTest, AddAlias) {
    cmdHistory->clearHistory();
    cmdHistory->addAlias("list", "ls -l");
    cmdHistory->executeAlias("list");
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printHistory();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("ls -l") != std::string::npos);
}

TEST_F(CommandHistoryTest, DeleteCommand) {
    cmdHistory->clearHistory();
    cmdHistory->addCommand("ls -l");
    cmdHistory->addCommand("cd /home/user");
    cmdHistory->deleteCommand(0);
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printHistory();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("ls -l") == std::string::npos);
    EXPECT_TRUE(output.str().find("cd /home/user") != std::string::npos);
}

TEST_F(CommandHistoryTest, SortHistoryByTime) {
    cmdHistory->clearHistory();
    cmdHistory->addCommand("ls -l");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cmdHistory->addCommand("cd /home/user");
    cmdHistory->sortHistoryByTime();
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printHistory();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("ls -l") != std::string::npos);
    EXPECT_TRUE(output.str().find("cd /home/user") != std::string::npos);
}

TEST_F(CommandHistoryTest, PrintFrequencyReport) {
    cmdHistory->clearHistory();
    cmdHistory->addCommand("ls -l");
    cmdHistory->addCommand("ls -l");
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        cmdHistory->printFrequencyReport();
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("ls -l: 2") != std::string::npos);
}

TEST_F(CommandHistoryTest, FilterHistoryByTime) {
    cmdHistory->clearHistory();
    cmdHistory->addCommand("ls -l");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    cmdHistory->addCommand("cd /home/user");
    std::time_t now = std::time(nullptr);
    std::time_t oneSecondAgo = now - 1;
    std::ostringstream output;
    {
        std::streambuf* old = std::cout.rdbuf(output.rdbuf());
        cmdHistory->filterHistoryByTime(oneSecondAgo, now);
        std::cout.rdbuf(old);
    }
    EXPECT_TRUE(output.str().find("cd /home/user") != std::string::npos);
    EXPECT_TRUE(output.str().find("ls -l") == std::string::npos);
}