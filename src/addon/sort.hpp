#ifndef LITHIUM_COMPONENTS_SORT_HPP
#define LITHIUM_COMPONENTS_SORT_HPP

#include <string>
#include <vector>

namespace lithium {
[[nodiscard("result is discarded")]] std::vector<std::string>
resolveDependencies(const std::vector<std::string>& directories);
}

#endif