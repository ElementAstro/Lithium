#include "atom/experiment/static_vector.hpp"

#include <iostream>
#include <string>

int main() {
    // 创建一个容量为5的StaticVector,存储int类型
    StaticVector<int, 5> vec;

    // 使用push_back添加元素
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    // 使用emplace_back添加元素
    vec.emplace_back(4);
    vec.emplace_back(5);

    // 使用下标操作符访问元素
    std::cout << "vec[0]: " << vec[0] << std::endl;
    std::cout << "vec[1]: " << vec[1] << std::endl;

    // 使用at函数访问元素
    std::cout << "vec.at(2): " << vec.at(2) << std::endl;
    try {
        std::cout << "vec.at(5): " << vec.at(5) << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Out of range: " << e.what() << std::endl;
    }

    // 使用front和back函数
    std::cout << "vec.front(): " << vec.front() << std::endl;
    std::cout << "vec.back(): " << vec.back() << std::endl;

    // 使用size和capacity函数
    std::cout << "vec.size(): " << vec.size() << std::endl;
    std::cout << "vec.capacity(): " << vec.capacity() << std::endl;

    // 使用范围for循环遍历元素
    std::cout << "Elements in vec: ";
    for (int x : vec) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    // 使用初始化列表构造函数
    StaticVector<std::string, 3> str_vec{"Hello", "World", "!"};

    // 使用比较操作符
    StaticVector<int, 3> vec1{1, 2, 3};
    StaticVector<int, 3> vec2{1, 2, 3};
    StaticVector<int, 3> vec3{1, 2, 4};

    std::cout << "vec1 == vec2: " << (vec1 == vec2) << std::endl;
    std::cout << "vec1 == vec3: " << (vec1 == vec3) << std::endl;
    // std::cout << "vec1 <=> vec2: " << (vec1 <=> vec2) << std::endl;
    // std::cout << "vec1 <=> vec3: " << (vec1 <=> vec3) << std::endl;

    // 使用swap函数
    StaticVector<int, 3> vec4{1, 2, 3};
    StaticVector<int, 3> vec5{4, 5, 6};
    std::cout << "Before swap: ";
    std::cout << "vec4: ";
    for (int x : vec4) {
        std::cout << x << " ";
    }
    std::cout << "vec5: ";
    for (int x : vec5) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    swap(vec4, vec5);

    std::cout << "After swap: ";
    std::cout << "vec4: ";
    for (int x : vec4) {
        std::cout << x << " ";
    }
    std::cout << "vec5: ";
    for (int x : vec5) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    return 0;
}
