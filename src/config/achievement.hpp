#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace OpenAPT::AAchievement
{
    // 成就类
    class Achievement
    {
    private:
        std::string m_name;        // 成就名称
        std::string m_description; // 成就描述
        bool m_isCompleted;        // 是否已完成

    public:
        Achievement();                                                        // 默认构造函数
        Achievement(const std::string &name, const std::string &description); // 构造函数

        std::string getName() const;        // 获取成就名称
        std::string getDescription() const; // 获取成就描述

        bool isCompleted() const; // 判断成就是否已完成
        void markAsCompleted();   // 标记成就为已完成

        json to_json() const;                                         // 将成就转换为 JSON 格式
        static std::shared_ptr<Achievement> from_json(const json &j); // 从 JSON 数据中创建成就
    };

}

#endif // ACHIEVEMENT_H
