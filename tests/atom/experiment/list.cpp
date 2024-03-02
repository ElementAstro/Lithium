#include "atom/experiment/list.hpp"

#include <gtest/gtest.h>

// 测试Deque类的基本功能
TEST(DequeTest, BasicFunctions)
{
    Deque<int> deque;

    // 测试空队列
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.get_size(), 0);

    // 测试push_front和pop_front
    deque.push_front(1);
    deque.push_front(2);
    EXPECT_EQ(deque.get_size(), 2);
    EXPECT_EQ(deque.pop_front().value(), 2);
    EXPECT_EQ(deque.pop_front().value(), 1);

    // 测试push_back和pop_back
    deque.push_back(3);
    deque.push_back(4);
    EXPECT_EQ(deque.get_size(), 2);
    EXPECT_EQ(deque.pop_back().value(), 4);
    EXPECT_EQ(deque.pop_back().value(), 3);

    // 测试peek_front和peek_back
    deque.push_front(5);
    deque.push_back(6);
    EXPECT_EQ(deque.peek_front().value(), 5);
    EXPECT_EQ(deque.peek_back().value(), 6);

    // 测试clear
    deque.clear();
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.get_size(), 0);
}

// 测试find函数
TEST(DequeTest, FindFunction)
{
    Deque<std::string> deque;
    deque.push_back("apple");
    deque.push_back("banana");
    deque.push_back("orange");

    EXPECT_EQ(deque.find("banana").value(), 1);
    EXPECT_FALSE(deque.find("grape").has_value());
}

// 测试insert函数
TEST(DequeTest, InsertFunction)
{
    Deque<char> deque;
    deque.push_back('a');
    deque.push_back('c');

    deque.insert(1, 'b');
    EXPECT_EQ(deque.get_size(), 3);
    EXPECT_EQ(deque.peek_back().value(), 'c');
    EXPECT_EQ(deque.peek_front().value(), 'a');
}

// 测试remove_at函数
TEST(DequeTest, RemoveAtFunction)
{
    Deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    deque.remove_at(1);
    EXPECT_EQ(deque.get_size(), 2);
    EXPECT_EQ(deque.pop_front().value(), 1);
    EXPECT_EQ(deque.pop_back().value(), 3);
}

// 测试reverse_traversal函数
TEST(DequeTest, ReverseTraversalFunction)
{
    Deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    // 捕获标准输出
    testing::internal::CaptureStdout();
    deque.reverse_traversal();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "3 2 1 \n");
}

// 测试concatenate函数
TEST(DequeTest, ConcatenateFunction)
{
    Deque<int> deque1;
    deque1.push_back(1);
    deque1.push_back(2);

    Deque<int> deque2;
    deque2.push_back(3);
    deque2.push_back(4);

    deque1.concatenate(deque2);
    EXPECT_EQ(deque1.get_size(), 4);
    EXPECT_TRUE(deque2.empty());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
