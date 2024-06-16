#include "atom/task/task.hpp"
#include <gtest/gtest.h>


TEST(SimpleTaskTest, TestToJson) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    json expected = json({{"type", "merged"},
                          {"name", ""},
                          {"id", 0},
                          {"description", ""},
                          {"can_stop", true}});
    EXPECT_EQ(task.toJson(), expected);
}

TEST(SimpleTaskTest, TestGetResult) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    json expected = json({});
    EXPECT_EQ(task.getResult(), expected);
}

TEST(SimpleTaskTest, TestGetParamsTemplate) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    json expected = json({});
    EXPECT_EQ(task.getParamsTemplate(), expected);
}

TEST(SimpleTaskTest, TestSetParams) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    json params = json({{"key", "value"}});
    task.setParams(params);
    EXPECT_EQ(task.m_params, params);
}

TEST(SimpleTaskTest, TestGetId) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    EXPECT_EQ(task.getId(), 0);
}

TEST(SimpleTaskTest, TestSetId) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    int newId = 123;
    task.setId(newId);
    EXPECT_EQ(task.getId(), newId);
}

TEST(SimpleTaskTest, TestGetName) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    EXPECT_EQ(task.getName(), "");
}

TEST(SimpleTaskTest, TestSetName) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    std::string newName = "New Task";
    task.setName(newName);
    EXPECT_EQ(task.getName(), newName);
}

TEST(SimpleTaskTest, TestGetDescription) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    EXPECT_EQ(task.getDescription(), "");
}

TEST(SimpleTaskTest, TestSetDescription) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    std::string newDescription = "New Description";
    task.setDescription(newDescription);
    EXPECT_EQ(task.getDescription(), newDescription);
}

TEST(SimpleTaskTest, TestSetCanExecute) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    task.setCanExecute(false);
    EXPECT_FALSE(task.isExecutable());
}

TEST(SimpleTaskTest, TestIsExecutable) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    EXPECT_TRUE(task.isExecutable());
}

TEST(SimpleTaskTest, TestSetStopFunction) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    std::function<json(const json&)> stopFunction = [](const json&) {
        return json();
    };
    task.setStopFunction(stopFunction);
    EXPECT_EQ(task.m_stopFn, stopFunction);
}

TEST(SimpleTaskTest, TestGetStopFlag) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    EXPECT_FALSE(task.getStopFlag());
}

TEST(SimpleTaskTest, TestSetStopFlag) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    task.setStopFlag(true);
    EXPECT_TRUE(task.getStopFlag());
}

TEST(SimpleTaskTest, TestStop) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    task.stop();
    EXPECT_TRUE(task.getStopFlag());
}

TEST(SimpleTaskTest, TestValidateJsonValue) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    json data = json({{"key", "value"}});
    json templateValue = json({{"key", ""}});
    EXPECT_TRUE(task.validateJsonValue(data, templateValue));
}

TEST(SimpleTaskTest, TestValidateJsonString) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    std::string jsonString = R"({"key": "value"})";
    std::string templateString = R"({"key": ""})";
    EXPECT_TRUE(task.validateJsonString(jsonString, templateString));
}

TEST(SimpleTaskTest, TestExecute) {
    SimpleTask task([](const json&) { return json(); },
                    [](const json&) { return json(); }, json());
    json expected = json({{"type", "merged"},
                          {"name", ""},
                          {"id", 0},
                          {"description", ""},
                          {"can_stop", true}});
    EXPECT_EQ(task.execute(), expected);
}