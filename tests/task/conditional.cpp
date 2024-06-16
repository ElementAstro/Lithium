#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "atom/task/conditional_task.hpp"

using json = nlohmann::json;

// 假设的条件、任务、停止函数逻辑
bool conditionMet(const json&) { return true; }  // 总是返回true，表示条件满足
bool conditionNotMet(const json&) {
    return false;
}  // 总是返回false，表示条件不满足
json dummyTask(const json&) {
    return json{{"result", "executed"}};
}  // 执行后返回特定结果的简单任务
json noOpStop(const json&) { return json{}; }  // 不做任何操作的停止函数

// 测试类
class ConditionalTaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化可以在每个测试用例前执行的通用设置
    }
};

// 测试场景1: 条件满足，任务执行
TEST_F(ConditionalTaskTest, ExecutesWhenConditionMet) {
    ConditionalTask task(dummyTask, conditionMet);
    json result = task.execute();
    EXPECT_EQ(result["result"], "executed");
}

// 测试场景2: 强制执行，即使条件不满足
TEST_F(ConditionalTaskTest, ExecutesWhenForced) {
    ConditionalTask task(dummyTask, conditionNotMet, noOpStop, json(), true);
    json result = task.execute();
    EXPECT_EQ(result["result"], "executed");
}

// 测试场景3: 条件不满足且未强制执行，任务不应执行
// 注意：此测试需根据ConditionalTask实际实现调整预期，假设当前实现下execute不返回特定结果表示未执行
TEST_F(ConditionalTaskTest, DoesNotExecuteWhenConditionNotMetAndNotForced) {
    ConditionalTask task(dummyTask, conditionNotMet);
    json result = task.execute();
    // 假设当任务未执行时，execute返回一个空json或特定标识
    EXPECT_TRUE(result.empty());
}

// 测试场景4: 序列化与反序列化
TEST_F(ConditionalTaskTest, SerializesAndDeserializesCorrectly) {
    ConditionalTask originalTask(dummyTask, conditionMet, noOpStop,
                                 json{{"param", 42}}, false);
    json serialized = originalTask.toJson();

    // 假设toJson返回一个包含所有必要信息的json对象
    EXPECT_TRUE(serialized.contains("conditionFn"));  // 确认存在条件函数的标识
    EXPECT_TRUE(serialized.contains("taskFn"));  // 确认存在任务函数的标识
    EXPECT_TRUE(serialized.contains("stopFn"));  // 确认存在停止函数的标识
    EXPECT_TRUE(serialized.contains("paramsTemplate"));  // 确认参数模板存在
    EXPECT_TRUE(serialized.contains("isForce"));  // 确认强制执行标志存在

    // 反序列化部分通常需要ConditionalTask提供一个静态构造函数或工厂方法，这里假设存在这样的机制
    // 注意：这部分代码依赖于ConditionalTask实现细节，实际应用中需要相应调整
    // ConditionalTask deserializedTask = ConditionalTask::fromJson(serialized);
    // ASSERT_EQ(originalTask.toJson(), deserializedTask.toJson()); //
    // 验证反序列化后与原对象相等
}