# Benchmark Class Documentation

## Overview

The `Benchmark` class provides a comprehensive solution for benchmarking code performance. It offers features such as multiple iterations, warmup runs, asynchronous execution, and detailed performance metrics including CPU and memory statistics.

## Class Definition

```cpp
class Benchmark {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    struct Config {
        int minIterations = 10;
        double minDurationSec = 1.0;
        bool async = false;
        bool warmup = true;
        std::string exportFormat = "json";
        Config() {}
    } ATOM_ALIGNAS(64);

    struct MemoryStats {
        size_t currentUsage;
        size_t peakUsage;
    } ATOM_ALIGNAS(16);

    struct CPUStats {
        long long instructionsExecuted;
        long long cyclesElapsed;
        long long branchMispredictions;
        long long cacheMisses;
    } ATOM_ALIGNAS(32);

    Benchmark(std::string suiteName, std::string name, Config config = {});

    template <typename SetupFunc, typename Func, typename TeardownFunc>
    void run(SetupFunc&& setupFunc, Func&& func, TeardownFunc&& teardownFunc);

    static void printResults(const std::string& suite = "");
    static void exportResults(const std::string& filename);

    // ... (private members omitted for brevity)
};
```

## Constructor

```cpp
Benchmark(std::string suiteName, std::string name, Config config = {});
```

Creates a new `Benchmark` object.

- **Parameters:**
  - `suiteName`: Name of the benchmark suite.
  - `name`: Name of the specific benchmark.
  - `config`: Configuration settings for the benchmark (optional).

## Member Functions

### run

```cpp
template <typename SetupFunc, typename Func, typename TeardownFunc>
void run(SetupFunc&& setupFunc, Func&& func, TeardownFunc&& teardownFunc);
```

Runs the benchmark with the specified setup, main function, and teardown steps.

- **Parameters:**
  - `setupFunc`: Function to set up the benchmark environment.
  - `func`: Main function to benchmark.
  - `teardownFunc`: Function to clean up after the benchmark.

### printResults

```cpp
static void printResults(const std::string& suite = "");
```

Prints the results of all benchmarks or those in a specific suite.

- **Parameters:**
  - `suite`: Optional suite name to filter results (default: print all).

### exportResults

```cpp
static void exportResults(const std::string& filename);
```

Exports the benchmark results to a file.

- **Parameters:**
  - `filename`: Name of the file to export results to.

## Configuration (Config struct)

- `minIterations`: Minimum number of iterations to run (default: 10).
- `minDurationSec`: Minimum duration of the benchmark in seconds (default: 1.0).
- `async`: Whether to run the benchmark asynchronously (default: false).
- `warmup`: Whether to perform a warmup run (default: true).
- `exportFormat`: Format for exporting results (default: "json").

## Usage Examples

### Example 1: Basic Benchmark

```cpp
#include "benchmark.hpp"
#include <vector>
#include <algorithm>

void benchmarkSort() {
    Benchmark::Config config;
    config.minIterations = 100;
    config.minDurationSec = 2.0;

    BENCHMARK("Sorting", "std::sort",
        // Setup
        []() {
            std::vector<int> vec(10000);
            for (int i = 0; i < 10000; ++i) vec[i] = rand();
            return vec;
        },
        // Benchmark function
        [](std::vector<int>& vec) {
            std::sort(vec.begin(), vec.end());
            return vec.size();  // Return number of operations
        },
        // Teardown
        [](std::vector<int>&) {},
        config
    );
}

int main() {
    benchmarkSort();
    Benchmark::printResults();
    Benchmark::exportResults("benchmark_results.json");
    return 0;
}
```

### Example 2: Asynchronous Benchmark with Custom Config

```cpp
#include "benchmark.hpp"
#include <thread>

void benchmarkThreadCreation() {
    Benchmark::Config config;
    config.async = true;
    config.warmup = false;
    config.minIterations = 50;

    BENCHMARK("Threading", "Thread Creation",
        // Setup
        []() { return 0; },
        // Benchmark function
        [](int&) {
            std::thread t([]() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
            t.join();
            return 1;  // One thread created and joined
        },
        // Teardown
        [](int&) {},
        config
    );
}

int main() {
    benchmarkThreadCreation();
    Benchmark::printResults("Threading");
    return 0;
}
```

## Important Considerations

1. **Performance Impact**: The benchmark itself may have some overhead, especially when measuring very fast operations. Consider this when interpreting results.

2. **System Load**: External factors like system load can affect benchmark results. Try to run benchmarks on a system with minimal background activity for more consistent results.

3. **Compiler Optimizations**: Be aware that aggressive compiler optimizations might eliminate code that appears to have no observable effects. Ensure your benchmark functions have observable side effects if necessary.

4. **Warmup Runs**: The warmup feature is useful for triggering any lazy initializations or cache warming, which can provide more stable results for subsequent runs.

5. **Asynchronous Execution**: When using the async feature, be mindful of potential resource contention with other system processes.

6. **Memory and CPU Stats**: The accuracy of memory and CPU statistics may vary depending on the platform and available system APIs.

## Best Practices

1. **Representative Data**: Use data sets and operations that are representative of real-world usage in your benchmarks.

2. **Multiple Runs**: Run benchmarks multiple times and analyze the distribution of results to account for variability.

3. **Comparative Benchmarks**: When possible, benchmark different implementations of the same functionality to make relative comparisons.

4. **Granularity**: Choose an appropriate granularity for your benchmark. Extremely short operations might need to be repeated in a loop within the benchmark function for accurate timing.

5. **Environment Control**: Document and control the environment (hardware, OS, compiler settings) in which benchmarks are run for reproducibility.

6. **Result Analysis**: Use the provided statistics (average, median, standard deviation) to gain a comprehensive understanding of the performance characteristics.

## Conclusion

The `Benchmark` class provides a powerful and flexible tool for performance testing in C++. By offering features like warmup runs, asynchronous execution, and detailed CPU and memory statistics, it allows for comprehensive analysis of code performance. When used correctly, it can provide valuable insights for optimization efforts and performance comparisons.
