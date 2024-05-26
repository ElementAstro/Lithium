/*!
 * \file type_caster.hpp
 * \brief Auto type caster, for better dispatch
 * \author Max Qian <lightapt.com>
 * \date 2023-04-05
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TYPE_CASTER_HPP
#define ATOM_META_TYPE_CASTER_HPP

#include <any>
#include <functional>
#include <queue>
#include <set>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>

#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_set8.hpp"
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#include <unordered_set>
#endif

#include "atom/error/exception.hpp"
#include "type_info.hpp"

namespace atom::meta {
class TypeCaster {
public:
    using ConvertFunc = std::function<std::any(const std::any&)>;
    using ConvertMap = std::unordered_map<Type_Info, ConvertFunc>;

    TypeCaster() {
        type_name_map["int"] = user_type<int>();
        type_name_map["double"] = user_type<double>();
        type_name_map["std::string"] = user_type<std::string>();
        type_name_map[typeid(std::string).name()] = user_type<std::string>();

        register_type<int>(typeid(int).name());
        register_type<double>(typeid(double).name());
        register_type<std::string>(typeid(std::string).name());
    }

    static std::shared_ptr<TypeCaster> createShared() {
        return std::make_shared<TypeCaster>();
    }

    template <typename SourceType, typename DestinationType>
    void register_conversion(ConvertFunc func) {
        auto src_info = user_type<SourceType>();
        auto dest_info = user_type<DestinationType>();
        if (src_info == dest_info) {
            THROW_INVALID_ARGUMENT(
                "Source and destination types must be different.");
        }
        conversions[src_info][dest_info] = std::move(func);
        clear_cache();  // Clear cache because new conversion might affect
                        // existing paths
    }

    std::vector<std::any> convert(
        const std::vector<std::any>& input,
        const std::vector<std::string>& target_type_names) {
        if (input.size() != target_type_names.size()) {
            THROW_INVALID_ARGUMENT(
                "Input and target type names must be of the same length.");
        }

        std::vector<std::any> output;
        for (size_t i = 0; i < input.size(); ++i) {
            auto dest_info = user_type_by_name(target_type_names[i]);
            auto src_info = get_type_info(input[i].type().name());

            if (src_info.value() == dest_info) {
                output.push_back(input[i]);
                continue;
            }

            auto path = find_conversion_path(src_info.value(), dest_info);
            std::any result = input[i];
            for (size_t j = 0; j < path.size() - 1; ++j) {
                result = conversions[path[j]][path[j + 1]](result);
            }
            output.push_back(result);
        }
        return output;
    }

private:
    std::unordered_map<Type_Info, ConvertMap> conversions;
    std::unordered_map<std::string, std::vector<Type_Info>>
        conversion_paths_cache;
    std::unordered_map<std::string, Type_Info> type_name_map;

    // Helper to generate a unique key for caching purposes
    std::string make_cache_key(Type_Info src, Type_Info dst) {
        return src.bare_name() + std::string("->") + dst.bare_name();
    }

    // Clears cached paths
    void clear_cache() { conversion_paths_cache.clear(); }

    // Helper function to find conversion path
    std::vector<Type_Info> find_conversion_path(Type_Info src, Type_Info dst) {
        std::string cache_key = make_cache_key(src, dst);
        if (conversion_paths_cache.find(cache_key) !=
            conversion_paths_cache.end()) {
            return conversion_paths_cache[cache_key];
        }

        std::queue<std::vector<Type_Info>> paths;
        paths.push({src});

        std::unordered_set<Type_Info> visited;
        visited.insert(src);

        while (!paths.empty()) {
            auto current_path = paths.front();
            paths.pop();
            auto last = current_path.back();

            if (last == dst) {
                conversion_paths_cache[cache_key] = current_path;
                return current_path;
            }

            auto find_it = conversions.find(last);
            if (find_it != conversions.end()) {
                for (auto& next : find_it->second) {
                    if (!visited.count(next.first)) {
                        visited.insert(next.first);
                        auto new_path = current_path;
                        new_path.push_back(next.first);
                        paths.push(new_path);
                    }
                }
            }
        }

        THROW_RUNTIME_ERROR("No conversion path found for these types.");
    }
    Type_Info user_type_by_name(const std::string& name) {
        // Look up the type name in the map
        auto it = type_name_map.find(name);
        if (it != type_name_map.end()) {
            return it->second;
        } else {
            // If the type name is not found, throw an exception
            THROW_RUNTIME_ERROR("Unknown type name: " + name);
        }
    }
};
}  // namespace atom::meta

#endif
