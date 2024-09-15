#include "dependency.hpp"
#include "version.hpp"

#include <fstream>
#include <unordered_map>
#include <utility>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium {
void DependencyGraph::addNode(const Node& node, const Version& version) {
    LOG_F(INFO, "Adding node: {} with version: {}", node, version.toString());
    adjList_.try_emplace(node);
    incomingEdges_.try_emplace(node);
    nodeVersions_[node] = version;
    LOG_F(INFO, "Node {} added successfully.", node);
}

void DependencyGraph::addDependency(const Node& from, const Node& to,
                                    const Version& requiredVersion) {
    LOG_F(INFO, "Adding dependency from {} to {} with required version: {}",
          from, to, requiredVersion.toString());

    if (nodeVersions_.contains(to) && nodeVersions_[to] < requiredVersion) {
        LOG_F(ERROR,
              "Version requirement not satisfied for dependency {} -> {}", from,
              to);
        THROW_INVALID_ARGUMENT(
            "Version requirement not satisfied for dependency " + from +
            " -> " + to);
    }

    adjList_[from].insert(to);
    incomingEdges_[to].insert(from);
    LOG_F(INFO, "Dependency from {} to {} added successfully.", from, to);
}

void DependencyGraph::removeNode(const Node& node) {
    LOG_F(INFO, "Removing node: {}", node);

    adjList_.erase(node);
    incomingEdges_.erase(node);

    for (auto& [key, neighbors] : adjList_) {
        neighbors.erase(node);
    }
    for (auto& [key, sources] : incomingEdges_) {
        sources.erase(node);
    }

    LOG_F(INFO, "Node {} removed successfully.", node);
}

void DependencyGraph::removeDependency(const Node& from, const Node& to) {
    LOG_F(INFO, "Removing dependency from {} to {}", from, to);

    if (adjList_.contains(from)) {
        adjList_[from].erase(to);
    }
    if (incomingEdges_.contains(to)) {
        incomingEdges_[to].erase(from);
    }

    LOG_F(INFO, "Dependency from {} to {} removed successfully.", from, to);
}

auto DependencyGraph::hasCycle() const -> bool {
    LOG_F(INFO, "Checking for cycles in the dependency graph.");
    std::unordered_set<Node> visited;
    std::unordered_set<Node> recStack;

    for (const auto& [node, _] : adjList_) {
        if (hasCycleUtil(node, visited, recStack)) {
            LOG_F(ERROR, "Cycle detected in the graph.");
            return true;
        }
    }
    LOG_F(INFO, "No cycles detected.");
    return false;
}

auto DependencyGraph::topologicalSort() const
    -> std::optional<std::vector<Node>> {
    LOG_F(INFO, "Performing topological sort.");
    std::unordered_set<Node> visited;
    std::stack<Node> stack;

    for (const auto& [node, _] : adjList_) {
        if (!visited.contains(node)) {
            if (!topologicalSortUtil(node, visited, stack)) {
                LOG_F(ERROR, "Cycle detected during topological sort.");
                return std::nullopt;
            }
        }
    }

    std::vector<Node> sortedNodes;
    while (!stack.empty()) {
        sortedNodes.push_back(stack.top());
        stack.pop();
    }

    LOG_F(INFO, "Topological sort completed successfully.");
    return sortedNodes;
}

auto DependencyGraph::resolveDependencies(
    const std::vector<std::string>& directories) -> std::vector<std::string> {
    LOG_F(INFO, "Resolving dependencies for directories.");
    DependencyGraph graph;

    for (const auto& dir : directories) {
        std::string packageJsonPath = dir + "/package.json";
        std::string packageXmlPath = dir + "/package.xml";

        if (std::filesystem::exists(packageJsonPath)) {
            LOG_F(INFO, "Parsing package.json in directory: {}", dir);
            auto [package_name, deps] = parsePackageJson(packageJsonPath);
            graph.addNode(package_name, deps.at(package_name));

            for (const auto& dep : deps) {
                if (dep.first != package_name) {
                    graph.addNode(dep.first, dep.second);
                    graph.addDependency(package_name, dep.first, dep.second);
                }
            }
        }

        if (std::filesystem::exists(packageXmlPath)) {
            LOG_F(INFO, "Parsing package.xml in directory: {}", dir);
            auto [package_name, deps] = parsePackageXml(packageXmlPath);
            graph.addNode(package_name, deps.at(package_name));

            for (const auto& dep : deps) {
                if (dep.first != package_name) {
                    graph.addNode(dep.first, dep.second);
                    graph.addDependency(package_name, dep.first, dep.second);
                }
            }
        }
    }

    if (graph.hasCycle()) {
        LOG_F(ERROR, "Circular dependency detected.");
        return {};
    }

    auto sortedPackagesOpt = graph.topologicalSort();
    if (!sortedPackagesOpt) {
        LOG_F(ERROR, "Failed to sort packages.");
        return {};
    }

    LOG_F(INFO, "Dependencies resolved successfully.");
    return removeDuplicates(sortedPackagesOpt.value());
}

auto DependencyGraph::parsePackageJson(const std::string& path)
    -> std::pair<std::string, std::unordered_map<std::string, Version>> {
    std::ifstream file(path);
    if (!file.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open " + path);
    }

    json packageJson;
    try {
        file >> packageJson;
    } catch (const json::exception& e) {
        THROW_JSON_PARSE_ERROR("Error parsing JSON in " + path + ": " +
                               e.what());
    }

    if (!packageJson.contains("name")) {
        THROW_MISSING_ARGUMENT("Missing package name in " + path);
    }

    std::string packageName = packageJson["name"];
    std::unordered_map<std::string, Version> deps;

    if (packageJson.contains("dependencies")) {
        for (const auto& dep : packageJson["dependencies"].items()) {
            deps[dep.key()] = Version::parse(dep.value().get<std::string>());
        }
    }

    file.close();
    return {packageName, deps};
}

auto DependencyGraph::parsePackageXml(const std::string& path)
    -> std::pair<std::string, std::unordered_map<std::string, Version>> {
    XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open " + path);
    }

    XMLElement* root = doc.FirstChildElement("package");
    if (root == nullptr) {
        THROW_MISSING_ARGUMENT("Missing root element in " + path);
    }

    const char* packageName = root->FirstChildElement("name")->GetText();
    if (packageName == nullptr) {
        THROW_MISSING_ARGUMENT("Missing package name in " + path);
    }

    std::unordered_map<std::string, Version> deps;

    XMLElement* dependElement = root->FirstChildElement("depend");
    while (dependElement != nullptr) {
        const char* depName = dependElement->GetText();
        if (depName != nullptr) {
            deps[depName] = Version{};  // Assuming no version info in XML,
                                        // could extend if needed.
        }
        dependElement = dependElement->NextSiblingElement("depend");
    }

    return {packageName, deps};
}

}  // namespace lithium
