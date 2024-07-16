#include <gtest/gtest.h>
#include "addon/addons.hpp"

class AddonManagerTest : public ::testing::Test {
protected:
    void SetUp() override { manager = lithium::AddonManager::createShared(); }

    std::shared_ptr<lithium::AddonManager> manager;
};

TEST_F(AddonManagerTest, AddModule) {
    ASSERT_TRUE(manager->addModule("path/to/addon", "testAddon"));
    ASSERT_NE(manager->getModule("testAddon"), nullptr);
}

TEST_F(AddonManagerTest, RemoveModule) {
    manager->addModule("path/to/addon", "testAddon");
    ASSERT_TRUE(manager->removeModule("testAddon"));
    ASSERT_EQ(manager->getModule("testAddon"), nullptr);
}

TEST_F(AddonManagerTest, ResolveDependencies) {
    manager->addModule("path/to/addon1", "addon1");
    manager->addModule("path/to/addon2", "addon2");
    std::vector<std::string> resolvedDeps;
    std::vector<std::string> missingDeps;
    ASSERT_TRUE(
        manager->resolveDependencies("addon1", resolvedDeps, missingDeps));
}

TEST_F(AddonManagerTest, CheckMissingDependencies) {
    manager->addModule("path/to/addon1", "addon1");
    std::vector<std::string> resolvedDeps;
    std::vector<std::string> missingDeps;
    manager->resolveDependencies("addon1", resolvedDeps, missingDeps);
    ASSERT_TRUE(missingDeps.empty());
}
