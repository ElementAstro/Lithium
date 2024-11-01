#ifndef LITHIUM_ADDON_DEPENDENCY_HPP
#define LITHIUM_ADDON_DEPENDENCY_HPP

#include <functional>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "tinyxml2/tinyxml2.h"

#include "atom/type/json_fwd.hpp"
#include "version.hpp"

using json = nlohmann::json;
using namespace tinyxml2;

namespace lithium {
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

    auto hasCycleUtil(const Node& node, std::unordered_set<Node>& visited,
                      std::unordered_set<Node>& recStack) const -> bool;

    auto topologicalSortUtil(const Node& node,
                             std::unordered_set<Node>& visited,
                             std::stack<Node>& stack) const -> bool;

    void getAllDependenciesUtil(
        const Node& node, std::unordered_set<Node>& allDependencies) const;

    static auto removeDuplicates(const std::vector<Node>& input)
        -> std::vector<Node>;

    static auto parsePackageJson(const Node& path)
        -> std::pair<Node, std::unordered_map<Node, Version>>;

    static auto parsePackageXml(const Node& path)
        -> std::pair<Node, std::unordered_map<Node, Version>>;

    static auto parsePackageYaml(const std::string& path)
        -> std::pair<std::string, std::unordered_map<std::string, Version>>;

    void generatePackageYaml(const std::string& path) const;
};
}  // namespace lithium
#endif  // LITHIUM_ADDON_DEPENDENCY_HPP
