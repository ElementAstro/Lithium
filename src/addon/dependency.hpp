#ifndef LITHIUM_ADDON_DEPENDENCY_HPP
#define LITHIUM_ADDON_DEPENDENCY_HPP

#include <functional>
#include <optional>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>

namespace lithium {
class DependencyGraph {
public:
    using Node = std::string;

    void addNode(const Node& node);
    void addDependency(const Node& from, const Node& to);

    void removeNode(const Node& node);

    void removeDependency(const Node& from, const Node& to);

    std::vector<Node> getDependencies(const Node& node) const;

    std::vector<Node> getDependents(const Node& node) const;

    bool hasCycle() const;

    std::optional<std::vector<Node>> topologicalSort() const;

    std::unordered_set<Node> getAllDependencies(const Node& node) const;

    void loadNodesInParallel(
        std::function<void(const Node&)> loadFunction) const;

private:
    std::unordered_map<Node, std::unordered_set<Node>> adjList;
    std::unordered_map<Node, std::unordered_set<Node>> incomingEdges;

    bool hasCycleUtil(const Node& node, std::unordered_set<Node>& visited,
                      std::unordered_set<Node>& recStack) const;

    bool topologicalSortUtil(const Node& node,
                             std::unordered_set<Node>& visited,
                             std::stack<Node>& stack) const;

    void getAllDependenciesUtil(
        const Node& node, std::unordered_set<Node>& allDependencies) const;
};
}  // namespace lithium

#endif