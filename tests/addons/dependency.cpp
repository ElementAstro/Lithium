#include "addon/dependency.hpp"
#include "addon/version.hpp"

#include <gtest/gtest.h>

using namespace lithium;

class DependencyGraphTest : public ::testing::Test {
protected:
    DependencyGraph graph;

    void SetUp() override {
        // Optionally, setup code if needed before each test
    }

    void TearDown() override {
        // Optionally, teardown code if needed after each test
    }
};

TEST_F(DependencyGraphTest, AddNode) {
    Version version = Version::parse("1.0.0");
    graph.addNode("A", version);

    auto dependencies = graph.getDependencies("A");
    EXPECT_TRUE(dependencies.empty());

    auto dependents = graph.getDependents("A");
    EXPECT_TRUE(dependents.empty());
}

TEST_F(DependencyGraphTest, AddDependency) {
    Version v1 = Version::parse("1.0.0");
    Version v2 = Version::parse("2.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v2);

    graph.addDependency("A", "B", v2);

    auto dependencies = graph.getDependencies("A");
    EXPECT_EQ(dependencies.size(), 1);
    EXPECT_EQ(dependencies[0], "B");

    auto dependents = graph.getDependents("B");
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], "A");
}

TEST_F(DependencyGraphTest, RemoveNode) {
    Version v1 = Version::parse("1.0.0");
    Version v2 = Version::parse("2.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v2);
    graph.addDependency("A", "B", v2);

    graph.removeNode("B");

    auto dependencies = graph.getDependencies("A");
    EXPECT_TRUE(dependencies.empty());

    auto dependents = graph.getDependents("B");
    EXPECT_TRUE(dependents.empty());
}

TEST_F(DependencyGraphTest, RemoveDependency) {
    Version v1 = Version::parse("1.0.0");
    Version v2 = Version::parse("2.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v2);
    graph.addDependency("A", "B", v2);

    graph.removeDependency("A", "B");

    auto dependencies = graph.getDependencies("A");
    EXPECT_TRUE(dependencies.empty());

    auto dependents = graph.getDependents("B");
    EXPECT_TRUE(dependents.empty());
}

TEST_F(DependencyGraphTest, DetectCycle) {
    Version v1 = Version::parse("1.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v1);
    graph.addNode("C", v1);

    graph.addDependency("A", "B", v1);
    graph.addDependency("B", "C", v1);
    graph.addDependency("C", "A", v1);

    EXPECT_TRUE(graph.hasCycle());
}

TEST_F(DependencyGraphTest, DetectNoCycle) {
    Version v1 = Version::parse("1.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v1);
    graph.addNode("C", v1);

    graph.addDependency("A", "B", v1);
    graph.addDependency("B", "C", v1);

    EXPECT_FALSE(graph.hasCycle());
}

TEST_F(DependencyGraphTest, TopologicalSortNoCycle) {
    Version v1 = Version::parse("1.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v1);
    graph.addNode("C", v1);

    graph.addDependency("A", "B", v1);
    graph.addDependency("B", "C", v1);

    auto sortedNodes = graph.topologicalSort();
    ASSERT_TRUE(sortedNodes.has_value());
    EXPECT_EQ(sortedNodes->size(), 3);
    EXPECT_EQ((*sortedNodes)[0], "A");
    EXPECT_EQ((*sortedNodes)[1], "B");
    EXPECT_EQ((*sortedNodes)[2], "C");
}

TEST_F(DependencyGraphTest, TopologicalSortWithCycle) {
    Version v1 = Version::parse("1.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v1);
    graph.addNode("C", v1);

    graph.addDependency("A", "B", v1);
    graph.addDependency("B", "C", v1);
    graph.addDependency("C", "A", v1);

    auto sortedNodes = graph.topologicalSort();
    EXPECT_FALSE(sortedNodes.has_value());
}

TEST_F(DependencyGraphTest, GetAllDependencies) {
    Version v1 = Version::parse("1.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v1);
    graph.addNode("C", v1);
    graph.addNode("D", v1);

    graph.addDependency("A", "B", v1);
    graph.addDependency("A", "C", v1);
    graph.addDependency("B", "D", v1);

    auto allDependencies = graph.getAllDependencies("A");
    EXPECT_EQ(allDependencies.size(), 3);
    EXPECT_TRUE(allDependencies.contains("B"));
    EXPECT_TRUE(allDependencies.contains("C"));
    EXPECT_TRUE(allDependencies.contains("D"));
}

TEST_F(DependencyGraphTest, LoadNodesInParallel) {
    Version v1 = Version::parse("1.0.0");

    graph.addNode("A", v1);
    graph.addNode("B", v1);
    graph.addNode("C", v1);

    graph.addDependency("A", "B", v1);
    graph.addDependency("B", "C", v1);

    std::vector<std::string> loadedNodes;
    std::mutex mtx;

    auto loadFunction = [&](const DependencyGraph::Node& node) {
        std::lock_guard<std::mutex> lock(mtx);
        loadedNodes.push_back(node);
    };

    graph.loadNodesInParallel(loadFunction);

    EXPECT_EQ(loadedNodes.size(), 3);
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), "A") !=
                loadedNodes.end());
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), "B") !=
                loadedNodes.end());
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), "C") !=
                loadedNodes.end());
}

TEST_F(DependencyGraphTest, ResolveDependencies) {
    // Simulate the presence of directories and their package.json files
    std::vector<std::string> directories = {
        "dirA",
        "dirB",
        "dirC",
    };

    // Assuming the function parsePackageJson and Version::parse are correctly
    // implemented This part should mock or create actual files if necessary for
    // the test environment Here we are just testing the graph structure and
    // behavior, so no actual file parsing is done

    // We are calling resolveDependencies method and just testing basic behavior
    auto resolvedDeps = graph.resolveDependencies(directories);
    // Note: Actual resolvedDeps content depends on package.json, this is just a
    // basic check
    EXPECT_TRUE(resolvedDeps.empty());
}
