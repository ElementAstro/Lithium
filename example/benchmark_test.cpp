#include <cmath>
#include "benchmark.hpp"

// Example function to benchmark
void ExampleFunction() {
    volatile double result = 0.0;
    for (int i = 0; i < 1000; ++i) {
        result += std::sin(i);
    }
}

// Another example function to benchmark
void AnotherExampleFunction() {
    volatile double result = 0.0;
    for (int i = 0; i < 1000; ++i) {
        result += std::cos(i);
    }
}

int main() {
    // Benchmark ExampleFunction with 100 iterations
    BENCHMARK("ExampleFunction", ExampleFunction, 100);

    // Benchmark AnotherExampleFunction with 100 iterations
    BENCHMARK("AnotherExampleFunction", AnotherExampleFunction, 100);

    // Print the results of the benchmarks
    Benchmark::PrintResults();

    return 0;
}
