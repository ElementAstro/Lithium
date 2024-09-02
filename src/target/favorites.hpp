#ifndef LITHIUM_TARGET_FAVORITES_HPP
#define LITHIUM_TARGET_FAVORITES_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::target {

template <typename T>
concept Serializable = requires(T a) {
    { std::cout << a };  // 输出支持
    { a == a };          // 可比较
};

template <Serializable T>
class FavoritesManager {
private:
    std::vector<T> favorites;
    std::vector<T> backup;
    std::optional<std::vector<T>> undoBackup;

public:
    void addFavorite(const T& item);
    void removeFavorite(const T& item);
    void displayFavorites() const;
    void displayFavoriteByIndex(size_t index) const;
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    void sortFavorites();
    bool findFavorite(const T& item) const;
    void removeDuplicates();
    size_t countFavorites() const;
    void backupFavorites();
    void restoreFavorites();
    void clearFavorites();
    void undoLastOperation();
    std::optional<T> mostFrequentFavorite() const;

    // 新增功能
    void batchAddFavorites(const std::vector<T>& items);
    void analyzeFavorites() const;

private:
    void createUndoBackup();
};

template <Serializable T>
void FavoritesManager<T>::addFavorite(const T& item) {
    createUndoBackup();
    favorites.push_back(item);
    LOG_F(INFO, "Added to favorites: {}", item);
}

template <Serializable T>
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

template <Serializable T>
void FavoritesManager<T>::displayFavorites() const {
    if (favorites.empty()) {
        std::cout << "Favorites list is empty" << std::endl;
        return;
    }
    LOG_F(INFO, "Favorites list:");
    for (const auto& item : favorites) {
        LOG_F(INFO, "- {}", item);
    }
}

template <Serializable T>
void FavoritesManager<T>::displayFavoriteByIndex(size_t index) const {
    if (index < favorites.size()) {
        LOG_F(INFO, "Favorite at index {}: {}", index, favorites[index]);
    } else {
        THROW_OUT_OF_RANGE("Index out of range");
    }
}

template <Serializable T>
void FavoritesManager<T>::saveToFile(const std::string& filename) const {
    nlohmann::json j = favorites;
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);  // Pretty print with 4 spaces indent
        LOG_F(INFO, "Favorites list saved to file: {}", filename);
    } else {
        THROW_FAIL_TO_OPEN_FILE("Unable to open file: {}", filename);
    }
}

template <Serializable T>
void FavoritesManager<T>::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        createUndoBackup();
        favorites = j.get<std::vector<T>>();
        LOG_F(INFO, "Favorites list loaded from file: {}", filename);
    } else {
        THROW_FAIL_TO_OPEN_FILE("Unable to open file: {}", filename);
    }
}

template <Serializable T>
void FavoritesManager<T>::sortFavorites() {
    createUndoBackup();
    std::ranges::sort(favorites);
    LOG_F(INFO, "Favorites list sorted");
}

template <Serializable T>
bool FavoritesManager<T>::findFavorite(const T& item) const {
    return std::ranges::find(favorites, item) != favorites.end();
}

template <Serializable T>
void FavoritesManager<T>::removeDuplicates() {
    createUndoBackup();
    std::set<T> uniqueFavorites(favorites.begin(), favorites.end());
    favorites.assign(uniqueFavorites.begin(), uniqueFavorites.end());
    LOG_F(INFO, "Duplicates removed from favorites list");
}

template <Serializable T>
size_t FavoritesManager<T>::countFavorites() const {
    return favorites.size();
}

template <Serializable T>
void FavoritesManager<T>::backupFavorites() {
    backup = favorites;
    LOG_F(INFO, "Favorites list backed up");
}

template <Serializable T>
void FavoritesManager<T>::restoreFavorites() {
    if (!backup.empty()) {
        createUndoBackup();
        favorites = backup;
        LOG_F(INFO, "Favorites list restored from backup");
    } else {
        THROW_FAIL_TO_OPEN_FILE("No backup available");
    }
}

template <Serializable T>
void FavoritesManager<T>::clearFavorites() {
    createUndoBackup();
    favorites.clear();
    LOG_F(INFO, "Favorites list cleared");
}

template <Serializable T>
void FavoritesManager<T>::undoLastOperation() {
    if (undoBackup.has_value()) {
        favorites = undoBackup.value();
        undoBackup.reset();
        LOG_F(INFO, "Last operation undone");
    } else {
        THROW_FAIL_TO_OPEN_FILE("No operation to undo");
    }
}

template <Serializable T>
std::optional<T> FavoritesManager<T>::mostFrequentFavorite() const {
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

template <Serializable T>
void FavoritesManager<T>::createUndoBackup() {
    undoBackup = favorites;
}

template <Serializable T>
void FavoritesManager<T>::batchAddFavorites(const std::vector<T>& items) {
    createUndoBackup();
    favorites.insert(favorites.end(), items.begin(), items.end());
    LOG_F(INFO, "Batch added favorites.");
}

template <Serializable T>
void FavoritesManager<T>::analyzeFavorites() const {
    LOG_F(INFO, "Analyzing favorites...");

    std::unordered_map<T, size_t> frequencyMap;
    for (const auto& item : favorites) {
        frequencyMap[item]++;
    }

    for (const auto& [item, count] : frequencyMap) {
        LOG_F(INFO, "{} appears {} times.", item, count);
    }
}
}  // namespace lithium::target

#endif  // LITHIUM_TARGET_FAVORITES_HPP
