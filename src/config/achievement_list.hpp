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

    protected:

            void addAstronomyPhotographyAchievements() {
                addAchievement(std::make_shared<Achievement>("Space Explorer", "Visit and document at least 10 major observatories around the world, from Chile's Atacama Desert to Hawaii's Mauna Kea."));
                addAchievement(std::make_shared<Achievement>("Cosmic Photographer", "Capture and share 100 stunning astronomical images, featuring everything from ethereal nebulae to awe-inspiring galaxies."));
                addAchievement(std::make_shared<Achievement>("Night Sky Watcher", "Observe and identify at least 50 constellations in the night sky, from Orion the Hunter to Ursa Major."));
                addAchievement(std::make_shared<Achievement>("Celestial Navigator", "Learn to use a sextant and navigate by the stars, just like the sailors of old."));
                addAchievement(std::make_shared<Achievement>("Meteor Hunter", "Witness and record at least 10 meteor showers, including the Perseids and the Leonids."));
                addAchievement(std::make_shared<Achievement>("Solar System Tourist", "Visit each planet in our solar system and document your journey, from the blistering heat of Mercury to the icy depths of Neptune."));
                addAchievement(std::make_shared<Achievement>("Light Painter", "Create and photograph at least 10 light paintings using long exposures, using creative techniques to capture the beauty of the night sky."));
                addAchievement(std::make_shared<Achievement>("Time Lapse Artist", "Create and share at least 10 captivating time-lapse videos of celestial events, from the majestic movement of the stars to the breathtaking beauty of a total solar eclipse."));
                addAchievement(std::make_shared<Achievement>("Astrophotographer of the Year", "Submit your best astronomical images and win a major astrophotography competition, showcasing your skills and creativity."));
                addAchievement(std::make_shared<Achievement>("Star Tracker", "Build and calibrate your own star tracker for capturing longer exposures, allowing you to take sharp and stunning images of the night sky."));
                addAchievement(std::make_shared<Achievement>("Lunar Lander", "Document and share your journey to the Moon, from the excitement of liftoff to the thrill of stepping on the lunar surface."));
                addAchievement(std::make_shared<Achievement>("Space Station Visitor", "Visit and document the International Space Station on a private tour, experiencing life in space firsthand."));
                addAchievement(std::make_shared<Achievement>("Deep Space Explorer", "Photograph and share at least 10 deep space objects with a telescope or specialized equipment, from distant galaxies to exploding supernovae."));
                addAchievement(std::make_shared<Achievement>("Planetary Portraitist", "Create and share at least 10 stunning composite images of planets and moons, turning raw data into art."));
                addAchievement(std::make_shared<Achievement>("Eclipse Chaser", "Travel to at least 10 locations around the world to observe total solar eclipses, chasing the shadow of the Moon across the Earth's surface."));
                addAchievement(std::make_shared<Achievement>("Galaxy Hunter", "Photograph and share at least 50 different galaxies, from the Milky Way to the Andromeda Galaxy."));
                addAchievement(std::make_shared<Achievement>("Aurora Chaser", "Photograph and share at least 10 aurora borealis displays, capturing the ethereal beauty of the Northern Lights."));
                addAchievement(std::make_shared<Achievement>("Cosmic Philosopher", "Write and publish a book that explores the philosophical implications of space exploration, pondering the big questions of our place in the universe."));
                addAchievement(std::make_shared<Achievement>("Satellites Sleuth", "Document and track at least 50 different artificial satellites in orbit around Earth, uncovering the secrets of our space infrastructure."));
                addAchievement(std::make_shared<Achievement>("Cosmic Jewelry Designer", "Create and sell a line of jewelry inspired by celestial objects, turning the beauty of the night sky into wearable art."));
                addAchievement(std::make_shared<Achievement>("Space Race Historian", "Write and publish a detailed history of the Space Race, chronicling the competition between the US and the Soviet Union to reach the stars."));
                addAchievement(std::make_shared<Achievement>("Cosmic Artist", "Create and display an exhibit of space-themed art, showcasing the beauty and mystery of the universe."));
                addAchievement(std::make_shared<Achievement>("Sky Atlas Compiler", "Create and publish your own comprehensive sky atlas, charting the stars and constellations in exquisite detail."));
                addAchievement(std::make_shared<Achievement>("Amateur Astronomer Award", "Receive an award in recognition of your contributions to amateur astronomy, celebrating your passion and dedication for exploring the universe."));
                addAchievement(std::make_shared<Achievement>("Star Party Host", "Organize and host at least 10 successful star parties for the public, sharing your love of astronomy with others."));
                addAchievement(std::make_shared<Achievement>("Rocket Scientist", "Design and build your own model rocket from scratch, and launch it successfully, exploring the principles of rocket science."));
                addAchievement(std::make_shared<Achievement>("Science Fiction Writer", "Publish a novel that explores space travel or alien life, using your imagination to push the boundaries of what is possible."));
                addAchievement(std::make_shared<Achievement>("Stellar Cartographer", "Create and share a detailed map of our galaxy with astronomical landmarks, discovering the wonders of the night sky."));
                addAchievement(std::make_shared<Achievement>("Solar System Model Maker", "Create and display a scale model of our solar system, allowing others to see the beauty and complexity of our neighborhood in space."));
                addAchievement(std::make_shared<Achievement>("Astrobiology Pioneer", "Conduct original research into the possibility of extraterrestrial life, unlocking the secrets of the universe and the potential for life beyond Earth."));
            } 


    private:
        std::vector<std::shared_ptr<Achievement>> m_achievements; // 成就列表
        std::string m_filename;                                   // 成就 JSON 文件名
    };

}
#endif // ACHIEVEMENT_LIST_H