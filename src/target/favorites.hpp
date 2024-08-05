#ifndef FAVORITESMANAGER_HPP
#define FAVORITESMANAGER_HPP

#include <optional>
#include <string>
#include <vector>

#include "atom/type/json.hpp"

namespace lithium::target {
template <typename T>
class FavoritesManager {
private:
    std::vector<T> favorites;
    std::vector<T> backup;
    std::optional<std::vector<T>> undoBackup;

public:
    // Add to favorites
    void addFavorite(const T& item);

    // Remove from favorites
    void removeFavorite(const T& item);

    // Display all favorites
    void displayFavorites() const;

    // Display favorite by index
    void displayFavoriteByIndex(size_t index) const;

    // Save favorites to JSON file
    void saveToFile(const std::string& filename) const;

    // Load favorites from JSON file
    void loadFromFile(const std::string& filename);

    // Sort favorites
    void sortFavorites();

    // Find favorite
    bool findFavorite(const T& item) const;

    // Remove duplicates
    void removeDuplicates();

    // Count favorites
    size_t countFavorites() const;

    // Backup favorites
    void backupFavorites();

    // Restore favorites from backup
    void restoreFavorites();

    // Clear all favorites
    void clearFavorites();

    // Undo last operation
    void undoLastOperation();

    // Get the most frequent favorite
    std::optional<T> mostFrequentFavorite() const;

private:
    // Create undo backup
    void createUndoBackup();
};
}  // namespace lithium::target

#include "favorites.tpp"

#endif  // FAVORITESMANAGER_HPP
