#include "sort.hpp"

#include <fstream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {
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
        q.push(node);
    }

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();
        sorted_packages.push_back(current);
        visited.insert(current);

        for (const auto& neighbor : dependency_graph[current]) {
            if (visited.count(neighbor)) {
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
