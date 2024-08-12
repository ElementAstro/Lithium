#ifndef FAVORITES_MANAGER_TPP
#define FAVORITES_MANAGER_TPP

#include <algorithm>
#include <fstream>
#include <ranges>
#include <set>

#include "favorites.hpp"

#include "atom/log/loguru.hpp"

namespace lithium::target {
template <typename T>
void FavoritesManager<T>::addFavorite(const T& item) {
    createUndoBackup();
    favorites.push_back(item);
    LOG_F(INFO, "Added to favorites: {}", item);
}

template <typename T>
void FavoritesManager<T>::removeFavorite(const T& item) {
    createUndoBackup();
    auto it = std::ranges::find(favorites, item);
    if (it != favorites.end()) {
        favorites.erase(it);
        LOG_F(INFO, "Removed from favorites: {}", item);
    } else {
        LOG_F(ERROR, "Item not found in favorites: {}", item);
    }
}

template <typename T>
void FavoritesManager<T>::displayFavorites() const {
    if (favorites.empty()) {
        LOG_F(WARNING, "Favorites list is empty");
        return;
    }
    LOG_F(INFO, "Favorites list:");
    for (const auto& item : favorites) {
        LOG_F(INFO, "- {}", item);
    }
}

template <typename T>
void FavoritesManager<T>::displayFavoriteByIndex(size_t index) const {
    if (index < favorites.size()) {
        LOG_F(INFO, "Favorite at index {}: {}", index, favorites[index]);
    } else {
        LOG_F(ERROR, "Index out of range");
    }
}

template <typename T>
void FavoritesManager<T>::saveToFile(const std::string& filename) const {
    nlohmann::json j = favorites;
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);  // Pretty print with 4 spaces indent
        file.close();
        LOG_F(INFO, "Favorites list saved to file: {}", filename);
    } else {
        LOG_F(ERROR, "Unable to open file: {}", filename);
    }
}

template <typename T>
void FavoritesManager<T>::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        createUndoBackup();
        favorites = j.get<std::vector<T>>();
        file.close();
        LOG_F(INFO, "Favorites list loaded from file: {}", filename);
    } else {
        LOG_F(ERROR, "Unable to open file: {}", filename);
    }
}

template <typename T>
void FavoritesManager<T>::sortFavorites() {
    createUndoBackup();
    std::ranges::sort(favorites);
    LOG_F(INFO, "Favorites list sorted");
}

template <typename T>
auto FavoritesManager<T>::findFavorite(const T& item) const -> bool {
    return std::ranges::find(favorites, item) != favorites.end();
}

template <typename T>
void FavoritesManager<T>::removeDuplicates() {
    createUndoBackup();
    std::set<T> uniqueFavorites(favorites.begin(), favorites.end());
    favorites.assign(uniqueFavorites.begin(), uniqueFavorites.end());
    LOG_F(INFO, "Duplicates removed from favorites list");
}

template <typename T>
auto FavoritesManager<T>::countFavorites() const -> size_t {
    return favorites.size();
}

template <typename T>
void FavoritesManager<T>::backupFavorites() {
    backup = favorites;
    LOG_F(INFO, "Favorites list backed up");
}

template <typename T>
void FavoritesManager<T>::restoreFavorites() {
    if (!backup.empty()) {
        createUndoBackup();
        favorites = backup;
        LOG_F(INFO, "Favorites list restored from backup");
    } else {
        LOG_F(WARNING, "No backup available");
    }
}

template <typename T>
void FavoritesManager<T>::clearFavorites() {
    createUndoBackup();
    favorites.clear();
    LOG_F(INFO, "Favorites list cleared");
}

template <typename T>
void FavoritesManager<T>::undoLastOperation() {
    if (undoBackup.has_value()) {
        favorites = undoBackup.value();
        undoBackup.reset();
        LOG_F(INFO, "Last operation undone");
    } else {
        LOG_F(WARNING, "No operation to undo");
    }
}

template <typename T>
auto FavoritesManager<T>::mostFrequentFavorite() const -> std::optional<T> {
    if (favorites.empty()) {
        return std::nullopt;
    }

    std::map<T, size_t> frequencyMap;
    for (const auto& item : favorites) {
        frequencyMap[item]++;
    }

    auto maxElement = std::max_element(frequencyMap.begin(), frequencyMap.end(),
                                       [](const auto& lhs, const auto& rhs) {
                                           return lhs.second < rhs.second;
                                       });

    return maxElement->first;
}

template <typename T>
void FavoritesManager<T>::createUndoBackup() {
    undoBackup = favorites;
}
}  // namespace lithium::target

#endif
