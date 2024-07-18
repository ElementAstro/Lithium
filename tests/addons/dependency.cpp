#include "addon/dependency.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace lithium;
namespace fs = std::filesystem;

class DependencyGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directories and package.json files for testing
        fs::create_directory(testDir1);
        fs::create_directory(testDir2);
        fs::create_directory(testDir3);
        fs::create_directory(testDir4);

        std::ofstream packageFile1(testDir1 + "/package.json");
        packageFile1 << R"({
            "name": "package1",
            "dependencies": {
                "package2": "1.0.0",
                "package3": "1.0.0"
            }
        })";
        packageFile1.close();

        std::ofstream packageFile2(testDir2 + "/package.json");
        packageFile2 << R"({
            "name": "package2",
            "dependencies": {
                "package4": "1.0.0"
            }
        })";
        packageFile2.close();

        std::ofstream packageFile3(testDir3 + "/package.json");
        packageFile3 << R"({
            "name": "package3",
            "dependencies": {}
        })";
        packageFile3.close();

        std::ofstream packageFile4(testDir4 + "/package.json");
        packageFile4 << R"({
            "name": "package4",
            "dependencies": {}
        })";
        packageFile4.close();
    }

    void TearDown() override {
        // Remove temporary directories and files after testing
        fs::remove_all(testDir1);
        fs::remove_all(testDir2);
        fs::remove_all(testDir3);
        fs::remove_all(testDir4);
    }

    std::string testDir1 = "test_dir1";
    std::string testDir2 = "test_dir2";
    std::string testDir3 = "test_dir3";
    std::string testDir4 = "test_dir4";
};

TEST_F(DependencyGraphTest, AddAndRemoveNode) {
    DependencyGraph graph;
    graph.addNode("A");
    ASSERT_EQ(graph.getDependencies("A").size(), 0);

    graph.removeNode("A");
    ASSERT_EQ(graph.getDependencies("A").size(), 0);
}

TEST_F(DependencyGraphTest, AddAndRemoveDependency) {
    DependencyGraph graph;
    graph.addNode("A");
    graph.addNode("B");
    graph.addDependency("A", "B");

    auto deps = graph.getDependencies("A");
    ASSERT_EQ(deps.size(), 1);
    ASSERT_EQ(deps[0], "B");

    graph.removeDependency("A", "B");
    deps = graph.getDependencies("A");
    ASSERT_EQ(deps.size(), 0);
}

TEST_F(DependencyGraphTest, HasCycle) {
    DependencyGraph graph;
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");
    ASSERT_FALSE(graph.hasCycle());

    graph.addDependency("C", "A");
    ASSERT_TRUE(graph.hasCycle());
}

TEST_F(DependencyGraphTest, TopologicalSort) {
    DependencyGraph graph;
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    auto sortedOpt = graph.topologicalSort();
    ASSERT_TRUE(sortedOpt.has_value());

    auto sorted = sortedOpt.value();
    ASSERT_EQ(sorted.size(), 3);
    ASSERT_EQ(sorted[0], "A");
    ASSERT_EQ(sorted[1], "B");
    ASSERT_EQ(sorted[2], "C");
}

TEST_F(DependencyGraphTest, GetAllDependencies) {
    DependencyGraph graph;
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    auto allDeps = graph.getAllDependencies("A");
    ASSERT_EQ(allDeps.size(), 2);
    ASSERT_TRUE(allDeps.contains("B"));
    ASSERT_TRUE(allDeps.contains("C"));
}

TEST_F(DependencyGraphTest, LoadNodesInParallel) {
    DependencyGraph graph;
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    std::vector<std::string> loadedNodes;
    graph.loadNodesInParallel([&](const auto& node) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100));  // Simulate work
        loadedNodes.push_back(node);
    });

    ASSERT_EQ(loadedNodes.size(), 3);
}

TEST_F(DependencyGraphTest, ResolveDependenciesSimple) {
    DependencyGraph graph;
    std::vector<std::string> directories = {testDir1, testDir2, testDir3,
                                            testDir4};

    // Resolve dependencies using the created package.json files
    auto sortedPackages = graph.resolveDependencies(directories);

    ASSERT_EQ(sortedPackages.size(), 4);
    // Ensure the order matches the dependency hierarchy
    ASSERT_EQ(sortedPackages[0], "package3");
    ASSERT_EQ(sortedPackages[1], "package4");
    ASSERT_EQ(sortedPackages[2], "package2");
    ASSERT_EQ(sortedPackages[3], "package1");
}

TEST_F(DependencyGraphTest, ResolveDependenciesWithCycle) {
    std::string testDir5 = "test_dir5";
    std::string testDir6 = "test_dir6";

    fs::create_directory(testDir5);
    fs::create_directory(testDir6);

    std::ofstream packageFile5(testDir5 + "/package.json");
    packageFile5 << R"({
        "name": "package5",
        "dependencies": {
            "package6": "1.0.0"
        }
    })";
    packageFile5.close();

    std::ofstream packageFile6(testDir6 + "/package.json");
    packageFile6 << R"({
        "name": "package6",
        "dependencies": {
            "package5": "1.0.0"
        }
    })";
    packageFile6.close();

    DependencyGraph graph;
    std::vector<std::string> directories = {testDir5, testDir6};

    // Resolve dependencies using the created package.json files
    auto sortedPackages = graph.resolveDependencies(directories);

    ASSERT_TRUE(sortedPackages.empty());

    fs::remove_all(testDir5);
    fs::remove_all(testDir6);
}

TEST_F(DependencyGraphTest, ResolveDependenciesNoDependencies) {
    std::string testDir7 = "test_dir7";

    fs::create_directory(testDir7);

    std::ofstream packageFile7(testDir7 + "/package.json");
    packageFile7 << R"({
        "name": "package7",
        "dependencies": {}
    })";
    packageFile7.close();

    DependencyGraph graph;
    std::vector<std::string> directories = {testDir7};

    // Resolve dependencies using the created package.json files
    auto sortedPackages = graph.resolveDependencies(directories);

    ASSERT_EQ(sortedPackages.size(), 1);
    ASSERT_EQ(sortedPackages[0], "package7");

    fs::remove_all(testDir7);
}
