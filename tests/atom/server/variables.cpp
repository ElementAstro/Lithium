#include <gtest/gtest.h>

// 引入 VariableRegistry 头文件
#include "atom/server/variables.hpp"

// 定义测试类
class VariableRegistryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_registry = std::make_shared<VariableRegistry>();
    }

private:
    std::shared_ptr<VariableRegistry> m_registry;
};

// 定义测试用例
TEST_F(VariableRegistryTest, RegisterAndGetValue)
{
    m_registry->RegisterVariable<int>("MyInt");

    // 设置变量值
    MyInt = 42;

    // 获取变量值
    int value = m_registry->GetVariable<int>("MyInt");

    // 断言变量值正确
    ASSERT_EQ(value, 42);
}

TEST_F(VariableRegistryTest, TypeCheck)
{
    // 注册 int 类型的变量
    REGISTER_VARIABLE(int, MyInt);

    // 尝试设置错误类型的值
    // 这里编译会出错，因为传入的值类型与注册时指定的类型不匹配
    // MyInt = "Hello"; // 编译错误

    // 获取变量值
    int value = m_registry->GetVariable<int>("MyInt");

    // 断言变量值未被改变
    ASSERT_EQ(value, 0);
}

TEST_F(VariableRegistryTest, RangeConstraint)
{
    // 注册 int 类型的变量，并设置范围限定为 [0, 100]
    m_registry->RegisterVariable<int>("MyLimitedInt");

    // 设置超出范围的值
    m_registry->SetVariable("MyLimitedInt", 100);

    // 获取变量值
    int value = m_registry->GetVariable<int>("MyLimitedInt");

    // 断言变量值在范围内
    ASSERT_EQ(value, 100);
}

// 运行测试
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
