#include <gtest/gtest.h>

#include "atom/system/crontab.hpp"

#include <fstream>
#include "atom/type/json.hpp"

using namespace nlohmann;

// Test fixture for CronManager
class CronManagerTest : public ::testing::Test {
protected:
    CronManager manager;

    void SetUp() override {
        // Set up any necessary preconditions here
    }

    void TearDown() override {
        // Clean up any necessary postconditions here
    }
};

// Test createCronJob method
TEST_F(CronManagerTest, CreateCronJob) {
    CronJob job{"* * * * *", "echo Hello"};
    EXPECT_TRUE(manager.createCronJob(job));
    auto jobs = manager.listCronJobs();
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].time_, "* * * * *");
    EXPECT_EQ(jobs[0].command_, "echo Hello");
}

// Test deleteCronJob method
TEST_F(CronManagerTest, DeleteCronJob) {
    CronJob job{"* * * * *", "echo Hello"};
    manager.createCronJob(job);
    EXPECT_TRUE(manager.deleteCronJob("echo Hello"));
    auto jobs = manager.listCronJobs();
    EXPECT_TRUE(jobs.empty());
}

// Test listCronJobs method
TEST_F(CronManagerTest, ListCronJobs) {
    CronJob job1{"* * * * *", "echo Hello"};
    CronJob job2{"0 0 * * *", "echo World"};
    manager.createCronJob(job1);
    manager.createCronJob(job2);
    auto jobs = manager.listCronJobs();
    ASSERT_EQ(jobs.size(), 2);
    EXPECT_EQ(jobs[0].time_, "* * * * *");
    EXPECT_EQ(jobs[0].command_, "echo Hello");
    EXPECT_EQ(jobs[1].time_, "0 0 * * *");
    EXPECT_EQ(jobs[1].command_, "echo World");
}

// Test exportToJSON method
TEST_F(CronManagerTest, ExportToJSON) {
    CronJob job{"* * * * *", "echo Hello"};
    manager.createCronJob(job);
    EXPECT_TRUE(manager.exportToJSON("cronjobs.json"));
    std::ifstream file("cronjobs.json");
    ASSERT_TRUE(file.is_open());
    json j;
    file >> j;
    ASSERT_EQ(j.size(), 1);
    EXPECT_EQ(j[0]["time"], "* * * * *");
    EXPECT_EQ(j[0]["command"], "echo Hello");
    file.close();
    std::remove("cronjobs.json");
}

// Test importFromJSON method
TEST_F(CronManagerTest, ImportFromJSON) {
    json j = {{{"time", "* * * * *"}, {"command", "echo Hello"}}};
    std::ofstream file("cronjobs.json");
    file << j.dump();
    file.close();
    EXPECT_TRUE(manager.importFromJSON("cronjobs.json"));
    auto jobs = manager.listCronJobs();
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].time_, "* * * * *");
    EXPECT_EQ(jobs[0].command_, "echo Hello");
    std::remove("cronjobs.json");
}

// Test updateCronJob method
TEST_F(CronManagerTest, UpdateCronJob) {
    CronJob job{"* * * * *", "echo Hello"};
    manager.createCronJob(job);
    CronJob newJob{"0 0 * * *", "echo World"};
    EXPECT_TRUE(manager.updateCronJob("echo Hello", newJob));
    auto jobs = manager.listCronJobs();
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].time_, "0 0 * * *");
    EXPECT_EQ(jobs[0].command_, "echo World");
}

// Test viewCronJob method
TEST_F(CronManagerTest, ViewCronJob) {
    CronJob job{"* * * * *", "echo Hello"};
    manager.createCronJob(job);
    auto viewedJob = manager.viewCronJob("echo Hello");
    EXPECT_EQ(viewedJob.time_, "* * * * *");
    EXPECT_EQ(viewedJob.command_, "echo Hello");
}

// Test searchCronJobs method
TEST_F(CronManagerTest, SearchCronJobs) {
    CronJob job1{"* * * * *", "echo Hello"};
    CronJob job2{"0 0 * * *", "echo World"};
    manager.createCronJob(job1);
    manager.createCronJob(job2);
    auto jobs = manager.searchCronJobs("Hello");
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].time_, "* * * * *");
    EXPECT_EQ(jobs[0].command_, "echo Hello");
}

// Test statistics method
TEST_F(CronManagerTest, Statistics) {
    CronJob job1{"* * * * *", "echo Hello"};
    CronJob job2{"0 0 * * *", "echo World"};
    manager.createCronJob(job1);
    manager.createCronJob(job2);
    EXPECT_EQ(manager.statistics(), 2);
}
