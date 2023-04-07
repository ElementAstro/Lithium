#include "achievement.hpp"
#include <memory>

namespace OpenAPT::AAchievement
{
    // 默认构造函数
    Achievement::Achievement()
        : m_name(""), m_description(""), m_isCompleted(false) {}

    // 构造函数
    Achievement::Achievement(const std::string& name, const std::string& description)
        : m_name(name), m_description(description), m_isCompleted(false) {}

    // 获取成就名称
    std::string Achievement::getName() const {
        return m_name;
    }

    // 获取成就描述
    std::string Achievement::getDescription() const {
        return m_description;
    }

    // 判断成就是否已完成
    bool Achievement::isCompleted() const {
        return m_isCompleted;
    }

    // 标记成就为已完成
    void Achievement::markAsCompleted() {
        m_isCompleted = true;
    }

    // 将成就转换为 JSON 格式
    json Achievement::to_json() const {
        json j;
        j["name"] = m_name;
        j["description"] = m_description;
        j["isCompleted"] = m_isCompleted;
        return j;
    }

    // 从 JSON 数据中创建成就
    std::shared_ptr<Achievement> Achievement::from_json(const json& j) {
        std::string name = j["name"];
        std::string description = j["description"];
        bool isCompleted = j["isCompleted"];
        auto achievement = std::make_shared<Achievement>(name, description);
        if (isCompleted) {
            achievement->markAsCompleted();
        }
        return achievement;
    }
}