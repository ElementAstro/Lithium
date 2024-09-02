#include "atom/algorithm/weight.hpp"

#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

int main() {
    // Sample weights
    std::vector<double> weights = {1.0, 2.0, 3.0, 4.0, 5.0};

    // Create a WeightSelector instance with default selection strategy
    atom::algorithm::WeightSelector<double> selector(weights);

    // Select a single weight based on the defined strategy
    size_t selectedIndex = selector.select();
    std::cout << "Selected index (default strategy): " << selectedIndex
              << " with weight: " << weights[selectedIndex] << std::endl;

    // Select multiple weights
    size_t n = 3;  // Number of selections
    auto chosenIndices = selector.selectMultiple(n);
    std::cout << "Selected indices for " << n << " selections: ";
    for (size_t index : chosenIndices) {
        std::cout << index << " (weight: " << weights[index] << "), ";
    }
    std::cout << std::endl;

    // Update a weight
    size_t updateIndex = 2;  // Change weight at index 2
    selector.updateWeight(updateIndex, 10.0);
    std::cout << "Updated weight at index " << updateIndex << " to 10.0."
              << std::endl;

    // Print current weights
    std::cout << "Current weights: ";
    selector.printWeights(std::cout);

    // Normalize weights
    selector.normalizeWeights();
    std::cout << "Normalized weights: ";
    selector.printWeights(std::cout);

    // Use TopHeavySelectionStrategy
    selector.setSelectionStrategy(
        std::make_unique<atom::algorithm::TopHeavySelectionStrategy<double>>());
    size_t heavySelectedIndex = selector.select();
    std::cout << "Selected index (TopHeavy strategy): " << heavySelectedIndex
              << " with weight: " << weights[heavySelectedIndex] << std::endl;

    // Add a new weight
    selector.addWeight(6.0);
    std::cout << "Added weight 6.0. New weights: ";
    selector.printWeights(std::cout);

    // Remove weight
    selector.removeWeight(0);  // remove the weight at index 0
    std::cout << "Removed weight at index 0. New weights: ";
    selector.printWeights(std::cout);

    // Get max and min weight indices
    size_t maxWeightIndex = selector.getMaxWeightIndex();
    size_t minWeightIndex = selector.getMinWeightIndex();
    std::cout << "Max weight index: " << maxWeightIndex
              << " (weight: " << weights[maxWeightIndex] << "), "
              << "Min weight index: " << minWeightIndex
              << " (weight: " << weights[minWeightIndex] << ")" << std::endl;

    return 0;
}
