# WeightSelector Class Documentation

## Overview

The `weight.hpp` file provides an implementation of a weight-based selection system, allowing for various selection strategies and weight manipulations. This implementation is part of the `atom::algorithm` namespace and is designed to work with arithmetic types.

## Namespace

All classes and functions are defined within the `atom::algorithm` namespace.

```cpp
namespace atom::algorithm {
    // ...
}
```

## WeightSelector Class

### Class Definition

```cpp
template <Arithmetic T>
class WeightSelector {
    // ... (member functions and nested classes)
};
```

The `WeightSelector` class is a template class that works with any arithmetic type `T`. It provides functionality for weight-based selection and various weight manipulation operations.

### Constructor

```cpp
explicit WeightSelector(std::span<const T> input_weights,
                        std::unique_ptr<SelectionStrategy> custom_strategy =
                            std::make_unique<DefaultSelectionStrategy>());
```

Constructs a `WeightSelector` object with the given weights and selection strategy.

- **Parameters:**
  - `input_weights`: A span of initial weights.
  - `custom_strategy`: A unique pointer to a custom selection strategy (optional, defaults to `DefaultSelectionStrategy`).
- **Usage:**

  ```cpp
  std::vector<double> weights = {1.0, 2.0, 3.0, 4.0};
  atom::algorithm::WeightSelector<double> selector(weights);
  ```

### Member Functions

#### select

```cpp
auto select() -> size_t;
```

Selects an index based on the current weights and selection strategy.

- **Returns:** The selected index.
- **Usage:**

  ```cpp
  size_t selected_index = selector.select();
  ```

#### selectMultiple

```cpp
auto selectMultiple(size_t n) -> std::vector<size_t>;
```

Selects multiple indices based on the current weights and selection strategy.

- **Parameters:**
  - `n`: The number of selections to make.
- **Returns:** A vector of selected indices.
- **Usage:**

  ```cpp
  std::vector<size_t> selected_indices = selector.selectMultiple(5);
  ```

#### updateWeight

```cpp
void updateWeight(size_t index, T new_weight);
```

Updates the weight at the specified index.

- **Parameters:**
  - `index`: The index of the weight to update.
  - `new_weight`: The new weight value.
- **Throws:** `OutOfRangeException` if the index is out of range.
- **Usage:**

  ```cpp
  selector.updateWeight(2, 5.0);
  ```

#### addWeight

```cpp
void addWeight(T new_weight);
```

Adds a new weight to the collection.

- **Parameters:**
  - `new_weight`: The weight to add.
- **Usage:**

  ```cpp
  selector.addWeight(6.0);
  ```

#### removeWeight

```cpp
void removeWeight(size_t index);
```

Removes the weight at the specified index.

- **Parameters:**
  - `index`: The index of the weight to remove.
- **Throws:** `OutOfRangeException` if the index is out of range.
- **Usage:**

  ```cpp
  selector.removeWeight(1);
  ```

#### normalizeWeights

```cpp
void normalizeWeights();
```

Normalizes all weights so they sum to 1.

- **Usage:**

  ```cpp
  selector.normalizeWeights();
  ```

#### applyFunctionToWeights

```cpp
void applyFunctionToWeights(std::invocable<T> auto&& func);
```

Applies a function to all weights.

- **Parameters:**
  - `func`: A function to apply to each weight.
- **Usage:**

  ```cpp
  selector.applyFunctionToWeights([](double w) { return w * 2; });
  ```

#### batchUpdateWeights

```cpp
void batchUpdateWeights(const std::vector<std::pair<size_t, T>>& updates);
```

Updates multiple weights in a single operation.

- **Parameters:**
  - `updates`: A vector of pairs containing indices and new weights.
- **Throws:** `OutOfRangeException` if any index is out of range.
- **Usage:**

  ```cpp
  std::vector<std::pair<size_t, double>> updates = {{0, 1.5}, {2, 3.5}};
  selector.batchUpdateWeights(updates);
  ```

#### getWeight

```cpp
[[nodiscard]] auto getWeight(size_t index) const -> std::optional<T>;
```

Retrieves the weight at the specified index.

- **Parameters:**
  - `index`: The index of the weight to retrieve.
- **Returns:** An optional containing the weight if the index is valid, or `std::nullopt` otherwise.
- **Usage:**

  ```cpp
  auto weight = selector.getWeight(1);
  if (weight) {
      std::cout << "Weight: " << *weight << std::endl;
  }
  ```

#### getMaxWeightIndex, getMinWeightIndex

```cpp
[[nodiscard]] auto getMaxWeightIndex() const -> size_t;
[[nodiscard]] auto getMinWeightIndex() const -> size_t;
```

Get the index of the maximum or minimum weight.

- **Returns:** The index of the maximum or minimum weight.
- **Usage:**

  ```cpp
  size_t max_index = selector.getMaxWeightIndex();
  size_t min_index = selector.getMinWeightIndex();
  ```

#### size, getWeights, getTotalWeight

```cpp
[[nodiscard]] auto size() const -> size_t;
[[nodiscard]] auto getWeights() const -> std::span<const T>;
[[nodiscard]] auto getTotalWeight() const -> T;
```

Utility functions to get information about the weights.

- **Usage:**

  ```cpp
  size_t num_weights = selector.size();
  auto weights_span = selector.getWeights();
  T total_weight = selector.getTotalWeight();
  ```

#### resetWeights, scaleWeights

```cpp
void resetWeights(const std::vector<T>& new_weights);
void scaleWeights(T factor);
```

Functions to reset or scale all weights.

- **Usage:**

  ```cpp
  std::vector<double> new_weights = {1.0, 2.0, 3.0};
  selector.resetWeights(new_weights);
  selector.scaleWeights(2.0);
  ```

#### getAverageWeight, printWeights

```cpp
[[nodiscard]] auto getAverageWeight() const -> T;
void printWeights(std::ostream& oss) const;
```

Utility functions to get the average weight or print all weights.

- **Usage:**

  ```cpp
  T avg_weight = selector.getAverageWeight();
  selector.printWeights(std::cout);
  ```

### Selection Strategies

The `WeightSelector` class supports different selection strategies through the `SelectionStrategy` interface:

1. `DefaultSelectionStrategy`: Uses uniform random selection based on weights.
2. `BottomHeavySelectionStrategy`: Biases selection towards lower indices.
3. `RandomSelectionStrategy`: Selects indices randomly, ignoring weights.
4. `TopHeavySelectionStrategy`: Biases selection towards higher indices.

You can set a custom strategy using:

```cpp
void setSelectionStrategy(std::unique_ptr<SelectionStrategy> new_strategy);
```

### WeightedRandomSampler

The `WeightedRandomSampler` class provides a method to sample multiple indices based on weights:

```cpp
class WeightedRandomSampler {
public:
    auto sample(std::span<const T> weights, size_t n) -> std::vector<size_t>;
};
```

#### sample

```cpp
auto sample(std::span<const T> weights, size_t n) -> std::vector<size_t>;
```

Samples `n` indices based on the given weights.

- **Parameters:**
  - `weights`: A span of weights to sample from.
  - `n`: The number of samples to take.
- **Returns:** A vector of sampled indices.
- **Usage:**

  ```cpp
  atom::algorithm::WeightSelector<double>::WeightedRandomSampler sampler;
  std::vector<double> weights = {1.0, 2.0, 3.0, 4.0};
  auto samples = sampler.sample(weights, 5);
  ```

## TopHeavySelectionStrategy

The `TopHeavySelectionStrategy` is an additional selection strategy that biases selection towards higher weights:

```cpp
template <Arithmetic T>
class TopHeavySelectionStrategy : public WeightSelector<T>::SelectionStrategy {
public:
    TopHeavySelectionStrategy();
    auto select(std::span<const T> cumulative_weights, T total_weight) -> size_t override;
};
```

This strategy can be used with the `WeightSelector` class to provide a bias towards selecting higher-weighted items.

## Example Usage

Here's an example demonstrating how to use the `WeightSelector` class with different strategies:

```cpp
#include "weight.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<double> weights = {1.0, 2.0, 3.0, 4.0};

    // Create a WeightSelector with default strategy
    atom::algorithm::WeightSelector<double> selector(weights);

    // Perform a selection
    size_t selected = selector.select();
    std::cout << "Selected index: " << selected << std::endl;

    // Change to bottom-heavy strategy
    selector.setSelectionStrategy(std::make_unique<atom::algorithm::WeightSelector<double>::BottomHeavySelectionStrategy>());

    // Perform multiple selections
    auto multiple_selections = selector.selectMultiple(5);
    std::cout << "Multiple selections: ";
    for (auto idx : multiple_selections) {
        std::cout << idx << " ";
    }
    std::cout << std::endl;

    // Update a weight
    selector.updateWeight(2, 5.0);

    // Normalize weights
    selector.normalizeWeights();

    // Print weights
    std::cout << "Normalized weights: ";
    selector.printWeights(std::cout);

    // Use WeightedRandomSampler
    atom::algorithm::WeightSelector<double>::WeightedRandomSampler sampler;
    auto samples = sampler.sample(selector.getWeights(), 10);
    std::cout << "Sampled indices: ";
    for (auto idx : samples) {
        std::cout << idx << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

## Best Practices

1. **Choose appropriate selection strategies**: Different selection strategies can significantly affect the behavior of your weight-based selection. Choose the strategy that best fits your use case.

2. **Keep weights normalized**: If you're frequently comparing or combining weights from different sources, consider keeping them normalized (sum to 1) using the `normalizeWeights()` method.

3. **Use batch updates**: When updating multiple weights at once, use `batchUpdateWeights()` for better performance, as it only recalculates cumulative weights once.

4. **Handle exceptions**: Methods like `updateWeight()` and `removeWeight()` can throw exceptions for out-of-range indices. Make sure to handle these exceptions in your code.

5. **Leverage SIMD operations**: If your application requires high-performance weight manipulations, consider using SIMD operations in custom weight manipulation functions.

6. **Use WeightedRandomSampler for multiple samples**: When you need to select multiple items based on weights, use the `WeightedRandomSampler` for efficiency, especially for large numbers of samples.

## Performance Considerations

- The `WeightSelector` class uses `std::exclusive_scan` for calculating cumulative weights, which can be efficiently parallelized on supporting hardware.
- Frequent weight updates can be costly due to the recalculation of cumulative weights. If your use case involves many updates, consider batching them or using a different data structure.
- The space complexity of the `WeightSelector` is O(n), where n is the number of weights, due to storing both the original weights and cumulative weights.
- The time complexity of selection operations is O(log n) due to the use of `std::ranges::upper_bound` in most selection strategies.

## Thread Safety

The `WeightSelector` class is not thread-safe by default. If you need to use it in a multi-threaded environment, you should implement external synchronization.

## Extensibility

The `WeightSelector` class is designed to be extensible:

- You can create custom selection strategies by inheriting from the `SelectionStrategy` base class.
- The `applyFunctionToWeights` method allows for custom weight transformations.
- You can extend the `WeightSelector` class itself to add domain-specific functionality.

## Limitations

- The current implementation assumes that weights are non-negative. Behavior with negative weights is undefined.
- The `WeightSelector` class doesn't currently support dynamic resizing of the weight vector for performance reasons. If you need to frequently add or remove weights, you might want to extend the class to support this use case more efficiently.
