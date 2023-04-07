#include "achievement_list.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <spdlog/spdlog.h>

using namespace std;

namespace OpenAPT::AAchievement
{
    // 默认构造函数
    AchievementList::AchievementList()
        : m_filename("") {}

    // 构造函数
    AchievementList::AchievementList(const std::string& filename)
        : m_filename(filename) {
        readFromFile();
    }

    // 添加成就
    void AchievementList::addAchievement(const std::shared_ptr<Achievement>& achievement) {
        m_achievements.push_back(achievement);
        spdlog::info("Achievement {} added to list.", achievement->getName());
        writeToFile();
    }

    // 根据名称删除成就
    void AchievementList::removeAchievementByName(const std::string& name) {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
            [&](const std::shared_ptr<Achievement>& achievement) {
                return achievement->getName() == name;
            });
        if (it != m_achievements.end()) {
            m_achievements.erase(it);
            spdlog::info("Achievement {} removed from list.", name);
            writeToFile();
        }
    }

    // 根据名称修改成就
    void AchievementList::modifyAchievementByName(const std::string& name, const std::shared_ptr<Achievement>& achievement) {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
            [&](const std::shared_ptr<Achievement>& a) {
                return a->getName() == name;
            });
        if (it != m_achievements.end()) {
            (*it) = achievement;
            spdlog::info("Achievement {} modified.", name);
            writeToFile();
        }
    }

    // 判断是否拥有某个成就
    bool AchievementList::hasAchievement(const std::string& name) const {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
            [&](const std::shared_ptr<Achievement>& achievement) {
                return achievement->getName() == name;
            });
        return it != m_achievements.end();
    }

    // 标记成就为已完成
    void AchievementList::completeAchievementByName(const std::string& name) {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
            [&](const std::shared_ptr<Achievement>& achievement) {
                return achievement->getName() == name;
            });
        if (it != m_achievements.end()) {
            (*it)->markAsCompleted();
            spdlog::info("Achievement {} marked as completed.", name);
            writeToFile();
        }
    }

    // 打印成就列表
    void AchievementList::printAchievements() const {
        spdlog::info("Achievements:");
        for (auto& achievement : m_achievements) {
            std::string status = achievement->isCompleted() ? "Completed" : "Incomplete";
            spdlog::info("\tName: {}, Description: {}, Status: {}", achievement->getName(), achievement->getDescription(), status);
        }
    }

    // 将成就列表写入文件
    void AchievementList::writeToFile() const {
        json j;
        for (auto& achievement : m_achievements) {
            j.push_back(achievement->to_json());
        }

        std::ofstream file(m_filename);
        if (!file.is_open()) {
            return;
        }

        file << std::setw(4) << j << std::endl;
        spdlog::info("Achievements written to file {}.", m_filename);
    }

    // 从文件中读取成就列表
    void AchievementList::readFromFile() {
        std::ifstream file(m_filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file.");
        }

        json j;
        try {
            file >> j;
        } catch (...) {
            throw std::runtime_error("Failed to parse JSON file.");
        }

        for (auto& item : j) {
            auto achievement = Achievement::from_json(item);
            m_achievements.push_back(achievement);
        }

        spdlog::info("Achievements read from file {}.", m_filename);
    }
}