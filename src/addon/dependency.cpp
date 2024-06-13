#include "dependency.hpp"

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace lithium {

void DependencyGraph::addNode(const DependencyGraph::Node& node) {
    adjList.try_emplace(node);
    incomingEdges.try_emplace(node);
}

void DependencyGraph::addDependency(const DependencyGraph::Node& from,
                                    const DependencyGraph::Node& to) {
    adjList[from].insert(to);
    incomingEdges[to].insert(from);
}

void DependencyGraph::removeNode(const DependencyGraph::Node& node) {
    adjList.erase(node);
    incomingEdges.erase(node);
    for (auto& [key, neighbors] : adjList) {
        neighbors.erase(node);
    }
    for (auto& [key, sources] : incomingEdges) {
        sources.erase(node);
    }
}

void DependencyGraph::removeDependency(const DependencyGraph::Node& from,
                                       const DependencyGraph::Node& to) {
    if (adjList.find(from) != adjList.end()) {
        adjList[from].erase(to);
    }
    if (incomingEdges.find(to) != incomingEdges.end()) {
        incomingEdges[to].erase(from);
    }
}

std::vector<DependencyGraph::Node> DependencyGraph::getDependencies(
    const DependencyGraph::Node& node) const {
    if (adjList.find(node) != adjList.end()) {
        return std::vector<DependencyGraph::Node>(adjList.at(node).begin(),
                                                  adjList.at(node).end());
    }
    return {};
}

std::vector<DependencyGraph::Node> DependencyGraph::getDependents(
    const DependencyGraph::Node& node) const {
    std::vector<DependencyGraph::Node> dependents;
    for (const auto& [key, neighbors] : adjList) {
        if (neighbors.find(node) != neighbors.end()) {
            dependents.push_back(key);
        }
    }
    return dependents;
}

bool DependencyGraph::hasCycle() const {
    std::unordered_set<DependencyGraph::Node> visited;
    std::unordered_set<DependencyGraph::Node> recStack;

    for (const auto& pair : adjList) {
        if (hasCycleUtil(pair.first, visited, recStack)) {
            return true;
        }
    }
    return false;
}

std::optional<std::vector<DependencyGraph::Node>>
DependencyGraph::topologicalSort() const {
    std::unordered_set<DependencyGraph::Node> visited;
    std::stack<DependencyGraph::Node> stack;
    for (const auto& pair : adjList) {
        if (visited.find(pair.first) == visited.end()) {
            if (!topologicalSortUtil(pair.first, visited, stack)) {
                return std::nullopt;  // Cycle detected
            }
        }
    }

    std::vector<DependencyGraph::Node> sortedNodes;
    while (!stack.empty()) {
        sortedNodes.push_back(stack.top());
        stack.pop();
    }
    return sortedNodes;
}

std::unordered_set<DependencyGraph::Node> DependencyGraph::getAllDependencies(
    const DependencyGraph::Node& node) const {
    std::unordered_set<DependencyGraph::Node> allDependencies;
    getAllDependenciesUtil(node, allDependencies);
    return allDependencies;
}

void DependencyGraph::loadNodesInParallel(
    std::function<void(const DependencyGraph::Node&)> loadFunction) const {
    std::queue<DependencyGraph::Node> readyQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::unordered_map<DependencyGraph::Node, int> inDegree;
    std::unordered_set<DependencyGraph::Node> loadedNodes;
    std::vector<std::thread> threads;
    bool done = false;

    // Initialize in-degree and ready queue
    for (const auto& [node, deps] : adjList) {
        inDegree[node] =
            incomingEdges.count(node) ? incomingEdges.at(node).size() : 0;
        if (inDegree[node] == 0) {
            readyQueue.push(node);
        }
    }

    auto worker = [&]() {
        while (true) {
            DependencyGraph::Node node;
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

                for (const auto& dep : adjList.at(node)) {
                    inDegree[dep]--;
                    if (inDegree[dep] == 0) {
                        readyQueue.push(dep);
                    }
                }

                if (readyQueue.empty() &&
                    loadedNodes.size() == adjList.size()) {
                    done = true;
                    cv.notify_all();
                } else {
                    cv.notify_all();
                }
            }
        }
    };

    int numThreads = std::thread::hardware_concurrency();
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

bool DependencyGraph::hasCycleUtil(
    const DependencyGraph::Node& node,
    std::unordered_set<DependencyGraph::Node>& visited,
    std::unordered_set<DependencyGraph::Node>& recStack) const {
    if (!visited.count(node)) {
        visited.insert(node);
        recStack.insert(node);

        for (const auto& neighbour : adjList.at(node)) {
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

bool DependencyGraph::topologicalSortUtil(
    const DependencyGraph::Node& node,
    std::unordered_set<DependencyGraph::Node>& visited,
    std::stack<DependencyGraph::Node>& stack) const {
    visited.insert(node);
    for (const auto& neighbour : adjList.at(node)) {
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
    const DependencyGraph::Node& node,
    std::unordered_set<DependencyGraph::Node>& allDependencies) const {
    if (adjList.find(node) != adjList.end()) {
        for (const auto& neighbour : adjList.at(node)) {
            if (allDependencies.find(neighbour) == allDependencies.end()) {
                allDependencies.insert(neighbour);
                getAllDependenciesUtil(neighbour, allDependencies);
            }
        }
    }
}
}  // namespace lithium
