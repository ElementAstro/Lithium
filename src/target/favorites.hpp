#ifndef LITHIUM_TARGET_FAVORITES_HPP
#define LITHIUM_TARGET_FAVORITES_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::target {

using json = nlohmann::json;

template <typename T>
concept Serializable = requires(T item) {
    { std::cout << item };  // 输出支持
    { item == item };       // 可比较
};

template <Serializable T>
class FavoritesManager {
private:
    std::vector<T> favorites_;
    std::vector<T> backup_;
    std::optional<std::vector<T>> undoBackup_;

public:
    void addFavorite(const T& item);
    void removeFavorite(const T& item);
    void displayFavorites() const;
    void displayFavoriteByIndex(size_t index) const;
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    void sortFavorites();
    [[nodiscard]] auto findFavorite(const T& item) const -> bool;
    void removeDuplicates();
    [[nodiscard]] auto countFavorites() const -> size_t;
    void backupFavorites();
    void restoreFavorites();
    void clearFavorites();
    void undoLastOperation();
    [[nodiscard]] auto mostFrequentFavorite() const -> std::optional<T>;

    // 新增功能
    void batchAddFavorites(const std::vector<T>& items);
    void analyzeFavorites() const;

private:
    void createUndoBackup();
};

template <Serializable T>
void FavoritesManager<T>::addFavorite(const T& item) {
    createUndoBackup();
    favorites_.push_back(item);
    LOG_F(INFO, "Added to favorites: {}", item);
}

template <Serializable T>
void FavoritesManager<T>::removeFavorite(const T& item) {
    createUndoBackup();
    auto iter = std::ranges::find(favorites_, item);
    if (iter != favorites_.end()) {
        favorites_.erase(iter);
        LOG_F(INFO, "Removed from favorites: {}", item);
    } else {
        LOG_F(ERROR, "Item not found in favorites: {}", item);
    }
}

template <Serializable T>
void FavoritesManager<T>::displayFavorites() const {
    if (favorites_.empty()) {
        std::cout << "Favorites list is empty" << std::endl;
        return;
    }
    LOG_F(INFO, "Favorites list:");
    for (const auto& item : favorites_) {
        LOG_F(INFO, "- {}", item);
    }
}

template <Serializable T>
void FavoritesManager<T>::displayFavoriteByIndex(size_t index) const {
    if (index < favorites_.size()) {
        LOG_F(INFO, "Favorite at index {}: {}", index, favorites_[index]);
    } else {
        THROW_OUT_OF_RANGE("Index out of range");
    }
}

template <Serializable T>
void FavoritesManager<T>::saveToFile(const std::string& filename) const {
    json jsonFavorites = favorites_;
    std::ofstream file(filename);
    if (file.is_open()) {
        file << jsonFavorites.dump(4);  // Pretty print with 4 spaces indent
        LOG_F(INFO, "Favorites list saved to file: {}", filename);
    } else {
        THROW_FAIL_TO_OPEN_FILE("Unable to open file: {}", filename);
    }
}

template <Serializable T>
void FavoritesManager<T>::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        json jsonFavorites;
        file >> jsonFavorites;
        createUndoBackup();
        favorites_ = jsonFavorites.get<std::vector<T>>();
        LOG_F(INFO, "Favorites list loaded from file: {}", filename);
    } else {
        THROW_FAIL_TO_OPEN_FILE("Unable to open file: {}", filename);
    }
}

template <Serializable T>
void FavoritesManager<T>::sortFavorites() {
    createUndoBackup();
    std::ranges::sort(favorites_);
    LOG_F(INFO, "Favorites list sorted");
}

template <Serializable T>
auto FavoritesManager<T>::findFavorite(const T& item) const -> bool {
    return std::ranges::find(favorites_, item) != favorites_.end();
}

template <Serializable T>
void FavoritesManager<T>::removeDuplicates() {
    createUndoBackup();
    std::set<T> uniqueFavorites(favorites_.begin(), favorites_.end());
    favorites_.assign(uniqueFavorites.begin(), uniqueFavorites.end());
    LOG_F(INFO, "Duplicates removed from favorites list");
}

template <Serializable T>
auto FavoritesManager<T>::countFavorites() const -> size_t {
    return favorites_.size();
}

template <Serializable T>
void FavoritesManager<T>::backupFavorites() {
    backup_ = favorites_;
    LOG_F(INFO, "Favorites list backed up");
}

template <Serializable T>
void FavoritesManager<T>::restoreFavorites() {
    if (!backup_.empty()) {
        createUndoBackup();
        favorites_ = backup_;
        LOG_F(INFO, "Favorites list restored from backup");
    } else {
        THROW_FAIL_TO_OPEN_FILE("No backup available");
    }
}

template <Serializable T>
void FavoritesManager<T>::clearFavorites() {
    createUndoBackup();
    favorites_.clear();
    LOG_F(INFO, "Favorites list cleared");
}

template <Serializable T>
void FavoritesManager<T>::undoLastOperation() {
    if (undoBackup_.has_value()) {
        favorites_ = undoBackup_.value();
        undoBackup_.reset();
        LOG_F(INFO, "Last operation undone");
    } else {
        THROW_FAIL_TO_OPEN_FILE("No operation to undo");
    }
}

template <Serializable T>
auto FavoritesManager<T>::mostFrequentFavorite() const -> std::optional<T> {
    if (favorites_.empty()) {
        return std::nullopt;
    }

    std::unordered_map<T, size_t> frequencyMap;
    for (const auto& item : favorites_) {
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
    undoBackup_ = favorites_;
}

template <Serializable T>
void FavoritesManager<T>::batchAddFavorites(const std::vector<T>& items) {
    createUndoBackup();
    favorites_.insert(favorites_.end(), items.begin(), items.end());
    LOG_F(INFO, "Batch added favorites.");
}

template <Serializable T>
void FavoritesManager<T>::analyzeFavorites() const {
    LOG_F(INFO, "Analyzing favorites...");

    std::unordered_map<T, size_t> frequencyMap;
    for (const auto& item : favorites_) {
        frequencyMap[item]++;
    }

    for (const auto& [item, count] : frequencyMap) {
        LOG_F(INFO, "{} appears {} times.", item, count);
    }
}
}  // namespace lithium::target

#endif  // LITHIUM_TARGET_FAVORITES_HPP
