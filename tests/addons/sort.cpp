#ifndef LITHIUM_ADDON_DEPENDENCY_HPP
#define LITHIUM_ADDON_DEPENDENCY_HPP

#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {

class DependencyGraph {
public:
    using Node = std::string;

    void addNode(const Node& node);
    void addDependency(const Node& from, const Node& to);
    void removeNode(const Node& node);
    void removeDependency(const Node& from, const Node& to);
    auto getDependencies(const Node& node) const -> std::vector<Node>;
    auto getDependents(const Node& node) const -> std::vector<Node>;
    auto hasCycle() const -> bool;
    auto topologicalSort() const -> std::optional<std::vector<Node>>;
    auto getAllDependencies(const Node& node) const -> std::unordered_set<Node>;
    void loadNodesInParallel(
        std::function<void(const Node&)> loadFunction) const;

private:
    std::unordered_map<Node, std::unordered_set<Node>> adjList_;
    std::unordered_map<Node, std::unordered_set<Node>> incomingEdges_;

    auto hasCycleUtil(const Node& node, std::unordered_set<Node>& visited,
                      std::unordered_set<Node>& recStack) const -> bool;
    auto topologicalSortUtil(const Node& node,
                             std::unordered_set<Node>& visited,
                             std::stack<Node>& stack) const -> bool;
    void getAllDependenciesUtil(
        const Node& node, std::unordered_set<Node>& allDependencies) const;
};

void DependencyGraph::addNode(const Node& node) {
    adjList_.try_emplace(node);
    incomingEdges_.try_emplace(node);
}

void DependencyGraph::addDependency(const Node& from, const Node& to) {
    adjList_[from].insert(to);
    incomingEdges_[to].insert(from);
}

void DependencyGraph::removeNode(const Node& node) {
    adjList_.erase(node);
    incomingEdges_.erase(node);
    for (auto& [key, neighbors] : adjList_) {
        neighbors.erase(node);
    }
    for (auto& [key, sources] : incomingEdges_) {
        sources.erase(node);
    }
}

void DependencyGraph::removeDependency(const Node& from, const Node& to) {
    if (adjList_.find(from) != adjList_.end()) {
        adjList_[from].erase(to);
    }
    if (incomingEdges_.find(to) != incomingEdges_.end()) {
        incomingEdges_[to].erase(from);
    }
}

auto DependencyGraph::getDependencies(const Node& node) const
    -> std::vector<DependencyGraph::Node> {
    if (adjList_.find(node) != adjList_.end()) {
        return {adjList_.at(node).begin(), adjList_.at(node).end()};
    }
    return {};
}

auto DependencyGraph::getDependents(const Node& node) const
    -> std::vector<DependencyGraph::Node> {
    std::vector<Node> dependents;
    for (const auto& [key, neighbors] : adjList_) {
        if (neighbors.find(node) != neighbors.end()) {
            dependents.push_back(key);
        }
    }
    return dependents;
}

auto DependencyGraph::hasCycle() const -> bool {
    std::unordered_set<Node> visited;
    std::unordered_set<Node> recStack;

    for (const auto& pair : adjList_) {
        if (hasCycleUtil(pair.first, visited, recStack)) {
            return true;
        }
    }
    return false;
}

auto DependencyGraph::topologicalSort() const
    -> std::optional<std::vector<DependencyGraph::Node>> {
    std::unordered_set<Node> visited;
    std::stack<Node> stack;
    for (const auto& pair : adjList_) {
        if (visited.find(pair.first) == visited.end()) {
            if (!topologicalSortUtil(pair.first, visited, stack)) {
                return std::nullopt;  // Cycle detected
            }
        }
    }

    std::vector<Node> sortedNodes;
    while (!stack.empty()) {
        sortedNodes.push_back(stack.top());
        stack.pop();
    }
    return sortedNodes;
}

std::unordered_set<DependencyGraph::Node> DependencyGraph::getAllDependencies(
    const Node& node) const {
    std::unordered_set<Node> allDependencies;
    getAllDependenciesUtil(node, allDependencies);
    return allDependencies;
}

void DependencyGraph::loadNodesInParallel(
    std::function<void(const Node&)> loadFunction) const {
    std::queue<Node> readyQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::unordered_map<Node, int> inDegree;
    std::unordered_set<Node> loadedNodes;
    std::vector<std::thread> threads;
    bool done = false;

    // Initialize in-degree and ready queue
    for (const auto& [node, deps] : adjList_) {
        inDegree[node] =
            (incomingEdges_.count(node) != 0U) ? incomingEdges_.at(node).size() : 0;
        if (inDegree[node] == 0) {
            readyQueue.push(node);
        }
    }

    auto worker = [&]() {
        while (true) {
            Node node;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] { return !readyQueue.empty() || done; });

                if (done && readyQueue.empty()) {
                    return;
                }

                node = readyQueue.front();
                readyQueue.pop();
            }

            loadFunction(node);

            {
                std::unique_lock<std::mutex> lock(mtx);
                loadedNodes.insert(node);

                for (const auto& dep : adjList_.at(node)) {
                    inDegree[dep]--;
                    if (inDegree[dep] == 0) {
                        readyQueue.push(dep);
                    }
                }

                if (readyQueue.empty() &&
                    loadedNodes.size() == adjList_.size()) {
                    done = true;
                    cv.notify_all();
                } else {
                    cv.notify_all();
                }
            }
        }
    };

    int numThreads = std::thread::hardware_concurrency();
    threads.reserve(numThreads);
for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

auto DependencyGraph::hasCycleUtil(const Node& node,
                                   std::unordered_set<Node>& visited,
                                   std::unordered_set<Node>& recStack) const -> bool {
    if (!visited.count(node)) {
        visited.insert(node);
        recStack.insert(node);

        for (const auto& neighbour : adjList_.at(node)) {
            if (!visited.count(neighbour) &&
                hasCycleUtil(neighbour, visited, recStack)) {
                return true;
            } else if (recStack.count(neighbour)) {
                return true;
            }
        }
    }
    recStack.erase(node);
    return false;
}

bool DependencyGraph::topologicalSortUtil(const Node& node,
                                          std::unordered_set<Node>& visited,
                                          std::stack<Node>& stack) const {
    visited.insert(node);
    for (const auto& neighbour : adjList_.at(node)) {
        if (visited.find(neighbour) == visited.end()) {
            if (!topologicalSortUtil(neighbour, visited, stack)) {
                return false;  // Cycle detected
            }
        }
    }
    stack.push(node);
    return true;
}

void DependencyGraph::getAllDependenciesUtil(
    const Node& node, std::unordered_set<Node>& allDependencies) const {
    if (adjList_.find(node) != adjList_.end()) {
        for (const auto& neighbour : adjList_.at(node)) {
            if (allDependencies.find(neighbour) == allDependencies.end()) {
                allDependencies.insert(neighbour);
                getAllDependenciesUtil(neighbour, allDependencies);
            }
        }
    }
}

std::vector<std::string> removeDuplicates(
    const std::vector<std::string>& input) {
    std::unordered_set<std::string> seen;
    std::vector<std::string> result;

    for (const auto& element : input) {
        if (seen.insert(element).second) {
            result.push_back(element);
        }
    }

    return result;
}

std::pair<std::string, std::vector<std::string>> parsePackageJson(
    const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        THROW_EXCEPTION("Failed to open " + path);
    }

    json package_json;
    try {
        file >> package_json;
    } catch (const json::exception& e) {
        THROW_EXCEPTION("Error parsing JSON in " + path + ": " + e.what());
    }

    if (!package_json.contains("name")) {
        THROW_EXCEPTION("Missing package name in " + path);
    }

    std::string package_name = package_json["name"];
    std::vector<std::string> deps;

    if (package_json.contains("dependencies")) {
        for (auto& dep : package_json["dependencies"].items()) {
            deps.push_back(dep.key());
        }
    }

    file.close();
    return {package_name, deps};
}

std::vector<std::string> resolveDependencies(
    const std::vector<std::string>& directories) {
    std::unordered_map<std::string, std::vector<std::string>> dependency_graph;
    std::unordered_map<std::string, int> indegree;
    std::vector<std::string> sorted_packages;
    std::unordered_set<std::string> visited;

    for (const auto& dir : directories) {
        std::string package_path = dir + "/package.json";
        auto [package_name, deps] = parsePackageJson(package_path);

        // 构建依赖图和入度表
        if (!dependency_graph.count(package_name)) {
            dependency_graph[package_name] = {};
            indegree[package_name] = 0;
        }

        for (const auto& dep : deps) {
            dependency_graph[dep].push_back(package_name);
            indegree[package_name]++;
        }
    }

    if (dependency_graph.empty()) {
        LOG_F(ERROR, "No packages found.");
        return {};
    }

    std::queue<std::string> q;
    for (const auto& [node, _] : dependency_graph) {
        if (indegree[node] == 0) {
            q.push(node);
        }
    }

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();
        sorted_packages.push_back(current);
        visited.insert(current);

        for (const auto& neighbor : dependency_graph[current]) {
            if (visited.contains(neighbor)) {
                LOG_F(WARNING,
                      "Circular dependency detected. Ignoring dependency from "
                      "{} to {}",
                      current, neighbor);
                continue;
            }
            indegree[neighbor]--;
            if (indegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    for (const auto& node : indegree) {
        if (node.second > 0) {
            LOG_F(WARNING, "Unresolved dependency for {}", node.first);
        }
    }

    if (sorted_packages.size() != dependency_graph.size()) {
        LOG_F(WARNING, "Some packages were not included in the load order.");
    }

    return removeDuplicates(sorted_packages);
}

}  // namespace lithium

#endif  // LITHIUM_ADDON_DEPENDENCY_HPP

#include <gtest/gtest.h>

using namespace lithium;

class DependencyGraphTest : public ::testing::Test {
protected:
    DependencyGraph graph;
};

TEST_F(DependencyGraphTest, AddNode) {
    graph.addNode("A");
    auto dependencies = graph.getDependencies("A");
    EXPECT_TRUE(dependencies.empty());
}

TEST_F(DependencyGraphTest, AddDependency) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addDependency("A", "B");

    auto dependencies = graph.getDependencies("A");
    EXPECT_EQ(dependencies.size(), 1);
    EXPECT_EQ(dependencies[0], "B");
}

TEST_F(DependencyGraphTest, RemoveNode) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addDependency("A", "B");

    graph.removeNode("B");
    auto dependencies = graph.getDependencies("A");
    EXPECT_TRUE(dependencies.empty());
}

TEST_F(DependencyGraphTest, RemoveDependency) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addDependency("A", "B");

    graph.removeDependency("A", "B");
    auto dependencies = graph.getDependencies("A");
    EXPECT_TRUE(dependencies.empty());
}

TEST_F(DependencyGraphTest, GetDependencies) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("A", "C");

    auto dependencies = graph.getDependencies("A");
    EXPECT_EQ(dependencies.size(), 2);
    EXPECT_NE(std::find(dependencies.begin(), dependencies.end(), "B"),
              dependencies.end());
    EXPECT_NE(std::find(dependencies.begin(), dependencies.end(), "C"),
              dependencies.end());
}

TEST_F(DependencyGraphTest, GetDependents) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("C", "B");

    auto dependents = graph.getDependents("B");
    EXPECT_EQ(dependents.size(), 2);
    EXPECT_NE(std::find(dependents.begin(), dependents.end(), "A"),
              dependents.end());
    EXPECT_NE(std::find(dependents.begin(), dependents.end(), "C"),
              dependents.end());
}

TEST_F(DependencyGraphTest, HasCycle) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    EXPECT_FALSE(graph.hasCycle());

    graph.addDependency("C", "A");
    EXPECT_TRUE(graph.hasCycle());
}

TEST_F(DependencyGraphTest, TopologicalSort) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    auto sorted = graph.topologicalSort();
    ASSERT_TRUE(sorted.has_value());
    EXPECT_EQ(sorted->size(), 3);
    EXPECT_LT(std::find(sorted->begin(), sorted->end(), "A"),
              std::find(sorted->begin(), sorted->end(), "B"));
    EXPECT_LT(std::find(sorted->begin(), sorted->end(), "B"),
              std::find(sorted->begin(), sorted->end(), "C"));
}

TEST_F(DependencyGraphTest, GetAllDependencies) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    auto allDeps = graph.getAllDependencies("A");
    EXPECT_EQ(allDeps.size(), 2);
    EXPECT_NE(allDeps.find("B"), allDeps.end());
    EXPECT_NE(allDeps.find("C"), allDeps.end());
}

TEST_F(DependencyGraphTest, LoadNodesInParallel) {
    graph.addNode("A");
    graph.addNode("B");
    graph.addNode("C");
    graph.addDependency("A", "B");
    graph.addDependency("B", "C");

    std::vector<std::string> loadOrder;
    auto loadFunction = [&](const std::string& node) {
        loadOrder.push_back(node);
    };

    graph.loadNodesInParallel(loadFunction);

    EXPECT_EQ(loadOrder.size(), 3);
    EXPECT_LT(std::find(loadOrder.begin(), loadOrder.end(), "A"),
              std::find(loadOrder.begin(), loadOrder.end(), "B"));
    EXPECT_LT(std::find(loadOrder.begin(), loadOrder.end(), "B"),
              std::find(loadOrder.begin(), loadOrder.end(), "C"));
}

TEST_F(DependencyGraphTest, ParsePackageJson) {
    auto [name, deps] = parsePackageJson("test_package.json");

    EXPECT_EQ(name, "test_package");
    EXPECT_EQ(deps.size(), 2);
    EXPECT_NE(std::find(deps.begin(), deps.end(), "dep1"), deps.end());
    EXPECT_NE(std::find(deps.begin(), deps.end(), "dep2"), deps.end());
}

TEST_F(DependencyGraphTest, ResolveDependencies) {
    std::vector<std::string> directories = {"dir1", "dir2", "dir3"};
    auto sortedPackages = resolveDependencies(directories);

    EXPECT_EQ(sortedPackages.size(), 3);
    EXPECT_NE(
        std::find(sortedPackages.begin(), sortedPackages.end(), "package1"),
        sortedPackages.end());
    EXPECT_NE(
        std::find(sortedPackages.begin(), sortedPackages.end(), "package2"),
        sortedPackages.end());
    EXPECT_NE(
        std::find(sortedPackages.begin(), sortedPackages.end(), "package3"),
        sortedPackages.end());
}
