# Perlin Noise Implementation Documentation

## Overview

The `perlin.hpp` file provides an implementation of Perlin Noise, a gradient noise algorithm used to generate procedural textures, terrains, and other natural-looking patterns. This implementation supports both CPU and OpenCL-based GPU computation.

## Namespace

The `PerlinNoise` class is defined within the `atom::algorithm` namespace.

```cpp
namespace atom::algorithm {
    // ...
}
```

## PerlinNoise Class

### Class Definition

```cpp
class PerlinNoise {
public:
    explicit PerlinNoise(unsigned int seed = std::default_random_engine::default_seed);
    ~PerlinNoise();

    template <std::floating_point T>
    [[nodiscard]] auto noise(T x, T y, T z) const -> T;

    template <std::floating_point T>
    [[nodiscard]] auto octaveNoise(T x, T y, T z, int octaves, T persistence) const -> T;

    [[nodiscard]] auto generateNoiseMap(int width, int height, double scale, int octaves,
                                        double persistence, double lacunarity,
                                        int seed = std::default_random_engine::default_seed) const
        -> std::vector<std::vector<double>>;

private:
    // ... (private members and methods)
};
```

### Constructor

```cpp
explicit PerlinNoise(unsigned int seed = std::default_random_engine::default_seed);
```

Constructs a PerlinNoise object with a specified seed for the random number generator.

- **Parameters:**
  - `seed`: Seed value for the random number generator (default is the default seed of `std::default_random_engine`).
- **Usage:**

  ```cpp
  atom::algorithm::PerlinNoise perlin(12345); // Create PerlinNoise with seed 12345
  ```

### Destructor

```cpp
~PerlinNoise();
```

Cleans up resources, including OpenCL resources if OpenCL is enabled.

### noise

```cpp
template <std::floating_point T>
[[nodiscard]] auto noise(T x, T y, T z) const -> T;
```

Generates 3D Perlin noise for given coordinates.

- **Template Parameters:**
  - `T`: A floating-point type.
- **Parameters:**
  - `x`, `y`, `z`: Coordinates for which to generate noise.
- **Returns:** A noise value in the range [0, 1].
- **Usage:**

  ```cpp
  atom::algorithm::PerlinNoise perlin;
  double value = perlin.noise(0.5, 1.0, 2.0);
  ```

### octaveNoise

```cpp
template <std::floating_point T>
[[nodiscard]] auto octaveNoise(T x, T y, T z, int octaves, T persistence) const -> T;
```

Generates 3D Perlin noise with multiple octaves for added detail.

- **Template Parameters:**
  - `T`: A floating-point type.
- **Parameters:**
  - `x`, `y`, `z`: Coordinates for which to generate noise.
  - `octaves`: Number of noise layers to combine.
  - `persistence`: Amplitude multiplier for each octave (usually < 1).
- **Returns:** A combined noise value in the range [0, 1].
- **Usage:**

  ```cpp
  atom::algorithm::PerlinNoise perlin;
  double value = perlin.octaveNoise(0.5, 1.0, 2.0, 4, 0.5);
  ```

### generateNoiseMap

```cpp
[[nodiscard]] auto generateNoiseMap(int width, int height, double scale, int octaves,
                                    double persistence, double lacunarity,
                                    int seed = std::default_random_engine::default_seed) const
    -> std::vector<std::vector<double>>;
```

Generates a 2D noise map using octave noise.

- **Parameters:**
  - `width`, `height`: Dimensions of the noise map.
  - `scale`: Scale of the noise (lower values create zoomed-out noise).
  - `octaves`: Number of noise layers to combine.
  - `persistence`: Amplitude multiplier for each octave.
  - `lacunarity`: Frequency multiplier for each octave (not used in current implementation).
  - `seed`: Seed for random number generation (default is the default seed of `std::default_random_engine`).
- **Returns:** A 2D vector representing the noise map.
- **Usage:**

  ```cpp
  atom::algorithm::PerlinNoise perlin;
  auto noiseMap = perlin.generateNoiseMap(256, 256, 50.0, 4, 0.5, 2.0, 12345);
  ```

## OpenCL Support

The Perlin Noise implementation includes support for OpenCL-based GPU computation when the `USE_OPENCL` macro is defined. When OpenCL is available, the `noise` method will automatically use GPU acceleration for improved performance.

### OpenCL-specific Methods

These methods are only available when `USE_OPENCL` is defined:

- `initializeOpenCL()`: Initializes OpenCL context and resources.
- `cleanupOpenCL()`: Cleans up OpenCL resources.
- `noiseOpenCL()`: Computes Perlin noise using OpenCL.

## SIMD Support

The implementation includes SIMD (Single Instruction, Multiple Data) optimizations for the fade function calculations when the `USE_SIMD` macro is defined. This can significantly improve performance on CPUs that support SIMD instructions.

## Example Usage

Here's an example demonstrating how to use the PerlinNoise class to generate a noise map and use it to create a simple terrain heightmap:

```cpp
#include "perlin.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    // Create a PerlinNoise object with a random seed
    atom::algorithm::PerlinNoise perlin(12345);

    // Generate a noise map
    int width = 100;
    int height = 100;
    double scale = 25.0;
    int octaves = 4;
    double persistence = 0.5;
    double lacunarity = 2.0;

    auto noiseMap = perlin.generateNoiseMap(width, height, scale, octaves, persistence, lacunarity);

    // Use the noise map to create a simple terrain heightmap
    std::vector<std::vector<char>> terrain(height, std::vector<char>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double value = noiseMap[y][x];
            if (value < 0.3) terrain[y][x] = '~';  // Water
            else if (value < 0.5) terrain[y][x] = '.';  // Sand
            else if (value < 0.7) terrain[y][x] = '*';  // Grass
            else if (value < 0.9) terrain[y][x] = '^';  // Hills
            else terrain[y][x] = 'M';  // Mountains
        }
    }

    // Print the terrain
    for (const auto& row : terrain) {
        for (char cell : row) {
            std::cout << cell;
        }
        std::cout << '\n';
    }

    return 0;
}
```
