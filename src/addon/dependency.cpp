#include "dependency.hpp"

#include <condition_variable>
#include <fstream>
#include <queue>
#include <thread>

namespace lithium {
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
        if (neighbors.contains(node)) {
            dependents.push_back(key);
        }
    }
    return dependents;
}

auto DependencyGraph::hasCycle() const -> bool {
    std::unordered_set<Node> visited;
    std::unordered_set<Node> recStack;

    for (const auto& [node, _] : adjList_) {
        if (hasCycleUtil(node, visited, recStack)) {
            return true;
        }
    }
    return false;
}

auto DependencyGraph::topologicalSort() const
    -> std::optional<std::vector<DependencyGraph::Node>> {
    std::unordered_set<Node> visited;
    std::stack<Node> stack;
    for (const auto& [node, _] : adjList_) {
        if (!visited.contains(node)) {
            if (!topologicalSortUtil(node, visited, stack)) {
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

auto DependencyGraph::getAllDependencies(const Node& node) const
    -> std::unordered_set<DependencyGraph::Node> {
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
    std::vector<std::jthread> threads;
    bool done = false;

    // Initialize in-degree and ready queue
    for (const auto& [node, deps] : adjList_) {
        inDegree[node] =
            incomingEdges_.contains(node) ? incomingEdges_.at(node).size() : 0;
        if (inDegree[node] == 0) {
            readyQueue.push(node);
        }
    }

    auto worker = [&]() {
        while (true) {
            Node node;
            {
                std::unique_lock lock(mtx);
                cv.wait(lock, [&] { return !readyQueue.empty() || done; });

                if (done && readyQueue.empty()) {
                    return;
                }

                node = readyQueue.front();
                readyQueue.pop();
            }

            loadFunction(node);

            {
                std::unique_lock lock(mtx);
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
}

auto DependencyGraph::hasCycleUtil(
    const Node& node, std::unordered_set<Node>& visited,
    std::unordered_set<Node>& recStack) const -> bool {
    if (!visited.contains(node)) {
        visited.insert(node);
        recStack.insert(node);

        for (const auto& neighbour : adjList_.at(node)) {
            if (!visited.contains(neighbour) &&
                hasCycleUtil(neighbour, visited, recStack)) {
                return true;
            }
            if (recStack.contains(neighbour)) {
                return true;
            }
        }
    }
    recStack.erase(node);
    return false;
}

auto DependencyGraph::topologicalSortUtil(
    const Node& node, std::unordered_set<Node>& visited,
    std::stack<Node>& stack) const -> bool {
    visited.insert(node);
    for (const auto& neighbour : adjList_.at(node)) {
        if (!visited.contains(neighbour)) {
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
    if (adjList_.contains(node)) {
        for (const auto& neighbour : adjList_.at(node)) {
            if (!allDependencies.contains(neighbour)) {
                allDependencies.insert(neighbour);
                getAllDependenciesUtil(neighbour, allDependencies);
            }
        }
    }
}

auto DependencyGraph::removeDuplicates(const std::vector<std::string>& input)
    -> std::vector<std::string> {
    std::unordered_set<std::string> seen;
    std::vector<std::string> result;

    for (const auto& element : input) {
        if (seen.insert(element).second) {
            result.push_back(element);
        }
    }

    return result;
}

auto DependencyGraph::parsePackageJson(const std::string& path)
    -> std::pair<std::string, std::vector<std::string>> {
    std::ifstream file(path);
    if (!file.is_open()) {
        // THROW_EXCEPTION("Failed to open " + path);
    }

    json packageJson;
    try {
        file >> packageJson;
    } catch (const json::exception& e) {
        // THROW_EXCEPTION("Error parsing JSON in " + path + ": " + e.what());
    }

    if (!packageJson.contains("name")) {
        // THROW_EXCEPTION("Missing package name in " + path);
    }

    std::string packageName = packageJson["name"];
    std::vector<std::string> deps;

    if (packageJson.contains("dependencies")) {
        for (const auto& dep : packageJson["dependencies"].items()) {
            deps.push_back(dep.key());
        }
    }

    file.close();
    return {packageName, deps};
}

auto DependencyGraph::resolveDependencies(
    const std::vector<std::string>& directories) -> std::vector<std::string> {
    DependencyGraph graph;

    for (const auto& dir : directories) {
        std::string packagePath = dir + "/package.json";
        auto [package_name, deps] = parsePackageJson(packagePath);

        graph.addNode(package_name);
        for (const auto& dep : deps) {
            graph.addNode(dep);
            graph.addDependency(
                dep, package_name);  // Ensure correct order of dependencies
        }
    }

    if (graph.hasCycle()) {
        // LOG_F(ERROR, "Circular dependency detected.");
        return {};
    }

    auto sortedPackagesOpt = graph.topologicalSort();
    if (!sortedPackagesOpt) {
        // LOG_F(ERROR, "Failed to sort packages.");
        return {};
    }

    return removeDuplicates(sortedPackagesOpt.value());
}
}  // namespace lithium
