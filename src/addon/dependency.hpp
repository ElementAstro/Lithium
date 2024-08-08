#ifndef LITHIUM_ADDON_DEPENDENCY_HPP
#define LITHIUM_ADDON_DEPENDENCY_HPP

#include <functional>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "atom/type/json_fwd.hpp"

using json = nlohmann::json;

namespace lithium {
class Version;
/**
 * @brief A class that represents a directed dependency graph.
 *
 * This class allows for managing nodes and their dependencies, detecting
 * cycles, performing topological sorting, and resolving dependencies for a
 * given set of directories.
 */
class DependencyGraph {
public:
    using Node = std::string;

    /**
     * @brief Adds a node to the dependency graph.
     *
     * @param node The name of the node to be added.
     */
    void addNode(const Node& node, const Version& version);

    /**
     * @brief Adds a directed dependency from one node to another.
     *
     * This establishes a relationship where 'from' depends on 'to'.
     *
     * @param from The node that has a dependency.
     * @param to The node that is being depended upon.
     */
    void addDependency(const Node& from, const Node& to,
                       const Version& requiredVersion);

    /**
     * @brief Removes a node from the dependency graph.
     *
     * This will also remove any dependencies associated with the node.
     *
     * @param node The name of the node to be removed.
     */
    void removeNode(const Node& node);

    /**
     * @brief Removes a directed dependency from one node to another.
     *
     * @param from The node that previously had a dependency.
     * @param to The node that is being removed from the dependency list.
     */
    void removeDependency(const Node& from, const Node& to);

    /**
     * @brief Retrieves the direct dependencies of a node.
     *
     * @param node The node for which to get dependencies.
     * @return A vector containing the names of the dependent nodes.
     */
    auto getDependencies(const Node& node) const -> std::vector<Node>;

    /**
     * @brief Retrieves the direct dependents of a node.
     *
     * @param node The node for which to get dependents.
     * @return A vector containing the names of the dependents.
     */
    auto getDependents(const Node& node) const -> std::vector<Node>;

    /**
     * @brief Checks if the dependency graph contains a cycle.
     *
     * @return True if there is a cycle in the graph, false otherwise.
     */
    auto hasCycle() const -> bool;

    /**
     * @brief Performs a topological sort of the nodes in the dependency graph.
     *
     * @return An optional vector containing the nodes in topological order, or
     * std::nullopt if a cycle is detected.
     */
    auto topologicalSort() const -> std::optional<std::vector<Node>>;

    /**
     * @brief Retrieves all dependencies of a node, including transitive
     * dependencies.
     *
     * @param node The node for which to get all dependencies.
     * @return A set containing all dependency node names.
     */
    auto getAllDependencies(const Node& node) const -> std::unordered_set<Node>;

    /**
     * @brief Loads nodes in parallel using a specified loading function.
     *
     * This function applies a provided function to each node concurrently.
     *
     * @param loadFunction The function to apply to each node.
     */
    void loadNodesInParallel(
        std::function<void(const Node&)> loadFunction) const;

    /**
     * @brief Resolves dependencies for a given list of directories.
     *
     * This function analyzes the specified directories and determines their
     * dependencies.
     *
     * @param directories A vector containing the paths of directories to
     * resolve.
     * @return A vector containing resolved dependency paths.
     */
    auto resolveDependencies(const std::vector<Node>& directories)
        -> std::vector<Node>;

private:
    std::unordered_map<Node, std::unordered_set<Node>>
        adjList_;  ///< Adjacency list representation of the graph.
    std::unordered_map<Node, std::unordered_set<Node>>
        incomingEdges_;  ///< Map to track incoming edges for each node.
    std::unordered_map<Node, Version>
        nodeVersions_;  ///< Map to track node versions.

    /**
     * @brief Utility function to check for cycles in the graph using DFS.
     *
     * @param node The current node being visited.
     * @param visited Set of visited nodes.
     * @param recStack Set of nodes currently in the recursion stack.
     * @return True if a cycle is detected, false otherwise.
     */
    auto hasCycleUtil(const Node& node, std::unordered_set<Node>& visited,
                      std::unordered_set<Node>& recStack) const -> bool;

    /**
     * @brief Utility function to perform DFS for topological sorting.
     *
     * @param node The current node being visited.
     * @param visited Set of visited nodes.
     * @param stack The stack to hold the topological order.
     * @return True if successful, false otherwise.
     */
    auto topologicalSortUtil(const Node& node,
                             std::unordered_set<Node>& visited,
                             std::stack<Node>& stack) const -> bool;

    /**
     * @brief Utility function to gather all dependencies of a node.
     *
     * @param node The node for which to collect dependencies.
     * @param allDependencies Set to hold all found dependencies.
     */
    void getAllDependenciesUtil(
        const Node& node, std::unordered_set<Node>& allDependencies) const;

    /**
     * @brief Removes duplicate entries from a vector of strings.
     *
     * @param input The input vector potentially containing duplicates.
     * @return A vector containing unique entries from the input.
     */
    static auto removeDuplicates(const std::vector<Node>& input)
        -> std::vector<Node>;

    /**
     * @brief Parses a package.json file to extract package name and
     * dependencies.
     *
     * @param path The path to the package.json file.
     * @return A pair containing the package name and its dependencies.
     */
    static auto parsePackageJson(const Node& path)
        -> std::pair<Node, std::unordered_map<Node, Version>>;
};

}  // namespace lithium
#endif  // LITHIUM_ADDON_DEPENDENCY_HPP
