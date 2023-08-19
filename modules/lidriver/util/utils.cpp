#include "utils.hpp"

#include <random>

int generateRandomNumber(int min, int max)
{
    // 使用随机设备作为种子
    std::random_device rd;
    // 使用 Mersenne Twister 引擎生成随机数
    std::mt19937 gen(rd());

    // 创建一个均匀分布的整数范围
    std::uniform_int_distribution<int> dis(min, max);

    // 生成随机数
    int randomNum = dis(gen);

    return randomNum;
}
