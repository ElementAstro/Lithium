#include <gtest/gtest.h>

#include "target/favorites.hpp"

using namespace lithium::target;

class FavoritesManagerTest : public testing::Test {
protected:
    FavoritesManager<std::string> manager;

    void SetUp() override {
        manager.clearFavorites();  // 确保每个测试前都是干净的状态
    }
};

// 测试添加收藏
TEST_F(FavoritesManagerTest, AddFavorite) {
    manager.addFavorite("Item1");
    EXPECT_EQ(manager.countFavorites(), 1);
    EXPECT_TRUE(manager.findFavorite("Item1"));
}

// 测试重复添加收藏
TEST_F(FavoritesManagerTest, AddFavoriteDuplicates) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item1");            // 重复添加
    EXPECT_EQ(manager.countFavorites(), 2);  // 应该是2个
}

// 测试移除收藏
TEST_F(FavoritesManagerTest, RemoveFavorite) {
    manager.addFavorite("Item1");
    manager.removeFavorite("Item1");
    EXPECT_EQ(manager.countFavorites(), 0);
    EXPECT_FALSE(manager.findFavorite("Item1"));
}

// 测试移除不存在的收藏
TEST_F(FavoritesManagerTest, RemoveNonExistentFavorite) {
    manager.addFavorite("Item1");
    manager.removeFavorite("Item2");         // 尝试移除不存在的项
    EXPECT_EQ(manager.countFavorites(), 1);  // 应该仍然是1个
}

// 测试清空收藏
TEST_F(FavoritesManagerTest, ClearFavorites) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item2");
    manager.clearFavorites();
    EXPECT_EQ(manager.countFavorites(), 0);
}

// 测试查看特定索引的收藏
TEST_F(FavoritesManagerTest, DisplayFavoriteByIndex) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item2");
    testing::internal::CaptureStdout();  // 捕获输出
    manager.displayFavoriteByIndex(1);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Item2"), std::string::npos);  // 确认输出包含 Item2
}

// 测试无效索引
TEST_F(FavoritesManagerTest, DisplayFavoriteByInvalidIndex) {
    testing::internal::CaptureStdout();  // 捕获原始输出
    manager.displayFavoriteByIndex(0);   // 无有效项
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Index out of range"), std::string::npos);
}

// 测试保存和加载
TEST_F(FavoritesManagerTest, SaveAndLoadFavorites) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item2");
    manager.saveToFile("test_favorites.json");

    FavoritesManager<std::string> newManager;
    newManager.loadFromFile("test_favorites.json");
    EXPECT_EQ(newManager.countFavorites(), 2);
    EXPECT_TRUE(newManager.findFavorite("Item1"));
    EXPECT_TRUE(newManager.findFavorite("Item2"));
}

// 测试查找最常用的收藏
TEST_F(FavoritesManagerTest, MostFrequentFavorite) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item1");
    manager.addFavorite("Item2");
    auto favorite = manager.mostFrequentFavorite();
    EXPECT_TRUE(favorite.has_value());  // 应该存在
    EXPECT_EQ(favorite.value(), "Item1");
}

// 测试冲突的备份和恢复
TEST_F(FavoritesManagerTest, BackupAndRestoreFavorites) {
    manager.addFavorite("Item1");
    manager.backupFavorites();
    manager.addFavorite("Item2");

    manager.restoreFavorites();
    EXPECT_EQ(manager.countFavorites(), 1);
    EXPECT_TRUE(manager.findFavorite("Item1"));
    EXPECT_FALSE(manager.findFavorite("Item2"));
}

// 测试去重
TEST_F(FavoritesManagerTest, RemoveDuplicates) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item1");  // 添加重复项
    manager.addFavorite("Item2");
    manager.removeDuplicates();
    EXPECT_EQ(manager.countFavorites(), 2);  // 应该去掉重复项
    EXPECT_TRUE(manager.findFavorite("Item1"));
    EXPECT_TRUE(manager.findFavorite("Item2"));
}

// 测试撤销操作
TEST_F(FavoritesManagerTest, UndoLastOperation) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item2");
    manager.removeFavorite("Item1");
    manager.undoLastOperation();  // 撤销最后一次操作，应该恢复 Item1
    EXPECT_EQ(manager.countFavorites(), 1);
    EXPECT_TRUE(manager.findFavorite("Item1"));
}

// 测试分析功能
TEST_F(FavoritesManagerTest, AnalyzeFavorites) {
    manager.addFavorite("Item1");
    manager.addFavorite("Item1");
    manager.addFavorite("Item2");
    testing::internal::CaptureStdout();
    manager.analyzeFavorites();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Item1 appears 2 times"), std::string::npos);
    EXPECT_NE(output.find("Item2 appears 1 times"), std::string::npos);
}
