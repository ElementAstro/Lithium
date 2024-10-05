# Simulated Annealing Algorithm Implementation

This document provides a detailed explanation of the Simulated Annealing algorithm implementation in C++, including the `SimulatedAnnealing` class and an example problem (Traveling Salesman Problem) that can be solved using this algorithm.

## Table of Contents

1. [Overview](#overview)
2. [SimulatedAnnealing Class](#simulatedannealing-class)
3. [Traveling Salesman Problem (TSP) Implementation](#traveling-salesman-problem-tsp-implementation)
4. [Usage Example](#usage-example)
5. [Optimization Techniques](#optimization-techniques)

## Overview

The Simulated Annealing algorithm is a probabilistic technique for approximating the global optimum of a given function. It is often used for optimization problems where finding an exact solution is impractical. This implementation provides a flexible and efficient way to apply Simulated Annealing to various problems.

## SimulatedAnnealing Class

### Class Template

```cpp
template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
class SimulatedAnnealing;
```

The `SimulatedAnnealing` class is templated to work with different problem types and solution representations.

### Constructor

```cpp
SimulatedAnnealing(ProblemType& problemInstance,
                   AnnealingStrategy coolingStrategy = AnnealingStrategy::EXPONENTIAL,
                   int maxIterations = K_DEFAULT_MAX_ITERATIONS,
                   double initialTemperature = K_DEFAULT_INITIAL_TEMPERATURE);
```

- `problemInstance`: Reference to the problem instance.
- `coolingStrategy`: The strategy for reducing temperature (default: EXPONENTIAL).
- `maxIterations`: Maximum number of iterations (default: 1000).
- `initialTemperature`: Starting temperature (default: 100.0).

### Public Methods

1. `setCoolingSchedule(AnnealingStrategy strategy)`

   - Sets the cooling schedule based on the specified strategy.

2. `setProgressCallback(std::function<void(int, double, const SolutionType&)> callback)`

   - Sets a callback function to report progress during optimization.

3. `setStopCondition(std::function<bool(int, double, const SolutionType&)> condition)`

   - Sets a condition to stop the optimization process early.

4. `optimize(int numThreads = 1) -> SolutionType`

   - Runs the optimization process with the specified number of threads.
   - Returns the best solution found.

5. `getBestEnergy() const -> double`
   - Returns the energy (cost) of the best solution found.

### Private Methods

1. `optimizeThread()`
   - The main optimization loop executed by each thread.

## Traveling Salesman Problem (TSP) Implementation

The TSP class demonstrates how to implement a specific problem for use with the SimulatedAnnealing class.

### Constructor

```cpp
TSP(const std::vector<std::pair<double, double>>& cities);
```

- `cities`: A vector of (x, y) coordinates representing city locations.

### Public Methods

1. `energy(const std::vector<int>& solution) const -> double`

   - Calculates the total distance of a given tour (solution).

2. `neighbor(const std::vector<int>& solution) -> std::vector<int>`

   - Generates a neighboring solution by swapping two cities.

3. `randomSolution() const -> std::vector<int>`
   - Generates a random initial solution.

## Usage Example

Here's an example of how to use the SimulatedAnnealing class with the TSP problem:

```cpp
#include "annealing.hpp"
#include <iostream>

int main() {
    // Define cities
    std::vector<std::pair<double, double>> cities = {
        {0, 0}, {1, 5}, {2, 2}, {3, 3}, {5, 1}
    };

    // Create TSP instance
    TSP tsp(cities);

    // Create SimulatedAnnealing instance
    SimulatedAnnealing<TSP, std::vector<int>> sa(tsp);

    // Set progress callback (optional)
    sa.setProgressCallback([](int iteration, double energy, const std::vector<int>& solution) {
        std::cout << "Iteration " << iteration << ": Energy = " << energy << std::endl;
    });

    // Run optimization
    auto bestSolution = sa.optimize(4);  // Use 4 threads

    // Print results
    std::cout << "Best solution energy: " << sa.getBestEnergy() << std::endl;
    std::cout << "Best tour: ";
    for (int city : bestSolution) {
        std::cout << city << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

## Optimization Techniques

1. **SIMD Instructions**: The `energy` calculation in the TSP class uses SIMD (Single Instruction, Multiple Data) instructions when available to parallelize distance calculations.

2. **Multi-threading**: The `optimize` method supports running multiple optimization threads in parallel.

3. **Flexible Cooling Strategies**: The implementation supports different cooling strategies (LINEAR, EXPONENTIAL, LOGARITHMIC) that can be easily switched.

4. **Early Stopping**: A custom stop condition can be set to terminate the optimization process early if certain criteria are met.

5. **Progress Tracking**: A callback function can be set to monitor the optimization progress in real-time.

These optimizations make the Simulated Annealing implementation efficient and adaptable to various problem types and computational environments.
