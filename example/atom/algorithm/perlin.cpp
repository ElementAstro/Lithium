#include <iostream>
#include <vector>

#include "atom/algorithm/perlin.hpp"

int main() {
    // Create a PerlinNoise object with a default seed
    atom::algorithm::PerlinNoise perlin;

    // Generate a noise value at a specific point (x, y, z)
    double x = 10.5, y = 20.5, z = 30.5;
    double noiseValue = perlin.noise(x, y, z);
    std::cout << "Noise Value at (" << x << ", " << y << ", " << z
              << "): " << noiseValue << std::endl;

    // Generate a noise map
    int width = 100;           // Width of the noise map
    int height = 100;          // Height of the noise map
    double scale = 50.0;       // Scale for the noise
    int octaves = 4;           // Number of octaves for the noise
    double persistence = 0.5;  // Persistence for the noise

    auto noiseMap = perlin.generateNoiseMap(width, height, scale, octaves,
                                            persistence, 0.5);

    // Output the first row of the noise map as an example
    std::cout << "Noise Map (first row):" << std::endl;
    for (const auto& value : noiseMap[0]) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    return 0;
}
