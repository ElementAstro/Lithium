#ifndef ACHIEVEMENT_LIST_H
#define ACHIEVEMENT_LIST_H

#include <vector>
#include <memory>
#include "nlohmann/json.hpp"

#include "achievement.hpp"

using json = nlohmann::json;

namespace OpenAPT::AAchievement
{

    // 成就列表类
    /**
     * @brief 成就列表类
     *
     */
    class AchievementList
    {
    public:
        /**
         * @brief 默认构造函数
         *
         */
        AchievementList();

        /**
         * @brief 构造函数
         *
         * @param filename 成就 JSON 文件名
         */
        explicit AchievementList(const std::string &filename);

        /**
         * @brief 添加成就
         *
         * @param achievement 成就指针
         */
        void addAchievement(const std::shared_ptr<Achievement> &achievement);

        /**
         * @brief 根据名称删除成就
         *
         * @param name 成就名称
         */
        void removeAchievementByName(const std::string &name);

        /**
         * @brief 根据名称修改成就
         *
         * @param name 成就名称
         * @param achievement 新的成就指针
         */
        void modifyAchievementByName(const std::string &name, const std::shared_ptr<Achievement> &achievement);

        /**
         * @brief 判断是否拥有某个成就
         *
         * @param name 成就名称
         * @return true 拥有该成就
         * @return false 不拥有该成就
         */
        bool hasAchievement(const std::string &name) const;

        /**
         * @brief 标记成就为已完成
         *
         * @param name 成就名称
         */
        void completeAchievementByName(const std::string &name);

        /**
         * @brief 打印成就列表
         *
         */
        void printAchievements() const;

        /**
         * @brief 将成就列表写入文件
         *
         */
        void writeToFile() const;

        /**
         * @brief 从文件中读取成就列表
         *
         */
        void readFromFile();

    private:
        std::vector<std::shared_ptr<Achievement>> m_achievements; // 成就列表
        std::string m_filename;                                   // 成就 JSON 文件名
    };

}
#endif // ACHIEVEMENT_LIST_H