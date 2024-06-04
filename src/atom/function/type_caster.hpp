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

    TypeCaster() { register_builtin_types(); }

    static std::shared_ptr<TypeCaster> createShared() {
        return std::make_shared<TypeCaster>();
    }

    template <typename SourceType, typename DestinationType>
    void register_conversion(ConvertFunc func) {
        auto src_info = user_type<SourceType>();
        auto dest_info = user_type<DestinationType>();
        register_type<SourceType>(src_info.bare_name());
        register_type<DestinationType>(dest_info.bare_name());
        if (src_info == dest_info) {
            THROW_INVALID_ARGUMENT(
                "Source and destination types must be different.");
        }
        conversions[src_info][dest_info] = std::move(func);
        clear_cache();  // Clear cache because new conversion might affect
                        // existing paths
    }

    template <typename SourceType, typename DestinationType>
    bool has_conversion() const {
        auto src_info = user_type<SourceType>();
        auto dest_info = user_type<DestinationType>();
        return has_conversion(src_info, dest_info);
    }

    std::vector<std::any> convert(
        const std::vector<std::any>& input,
        const std::vector<std::string>& target_type_names) const {
        if (input.size() != target_type_names.size()) {
            THROW_INVALID_ARGUMENT(
                "Input and target type names must be of the same length.");
        }

        std::vector<std::any> output;
        for (size_t i = 0; i < input.size(); ++i) {
            auto dest_info = user_type_by_name(target_type_names[i]);
            auto src_info = get_type_info(input[i].type().name());
            if (!src_info.has_value()) {
                THROW_INVALID_ARGUMENT("Type " + target_type_names[i] +
                                       " not found.");
            }

            if (src_info.value() == dest_info) {
                output.push_back(input[i]);
                continue;
            }

            auto path = find_conversion_path(src_info.value(), dest_info);
            std::any result = input[i];
            for (size_t j = 0; j < path.size() - 1; ++j) {
                result = conversions.at(path[j]).at(path[j + 1])(result);
            }
            output.push_back(result);
        }
        return output;
    }

    std::vector<std::string> get_registered_types() const {
        std::vector<std::string> type_names;
        for (const auto& [name, info] : type_name_map) {
            type_names.push_back(name);
        }
        return type_names;
    }

    template <typename T>
    void register_type(const std::string& name) {
        type_name_map[name] = user_type<T>();
        type_name_map[typeid(T).name()] = user_type<T>();
        detail::get_type_registry()[typeid(T).name()] = user_type<T>();
    }

private:
    std::unordered_map<Type_Info, ConvertMap> conversions;
    mutable std::unordered_map<std::string, std::vector<Type_Info>>
        conversion_paths_cache;
    std::unordered_map<std::string, Type_Info> type_name_map;

    void register_builtin_types() {
        register_type<int>("int");
        register_type<double>("double");
        register_type<std::string>("std::string");
    }

    bool has_conversion(Type_Info src, Type_Info dst) const {
        return conversions.find(src) != conversions.end() &&
               conversions.at(src).find(dst) != conversions.at(src).end();
    }

    // Helper to generate a unique key for caching purposes
    std::string make_cache_key(Type_Info src, Type_Info dst) const {
        return src.bare_name() + "->" + dst.bare_name();
    }

    // Clears cached paths
    void clear_cache() { conversion_paths_cache.clear(); }

    // Helper function to find conversion path
    std::vector<Type_Info> find_conversion_path(Type_Info src,
                                                Type_Info dst) const {
        std::string cache_key = make_cache_key(src, dst);
        if (conversion_paths_cache.find(cache_key) !=
            conversion_paths_cache.end()) {
            return conversion_paths_cache.at(cache_key);
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
                for (const auto& [next_type, _] : find_it->second) {
                    if (visited.insert(next_type).second) {
                        auto new_path = current_path;
                        new_path.push_back(next_type);
                        paths.push(std::move(new_path));
                    }
                }
            }
        }

        THROW_RUNTIME_ERROR("No conversion path found for these types.");
    }

    Type_Info user_type_by_name(const std::string& name) const {
        auto it = type_name_map.find(name);
        if (it != type_name_map.end()) {
            return it->second;
        } else {
            THROW_RUNTIME_ERROR("Unknown type name: " + name);
        }
    }
};

}  // namespace atom::meta

#endif
