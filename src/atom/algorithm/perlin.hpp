#ifndef ATOM_ALGORITHM_PERLIN_HPP
#define ATOM_ALGORITHM_PERLIN_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <numeric>
#include <random>
#include <span>
#include <vector>

namespace atom::algorithm {
class PerlinNoise {
public:
    explicit PerlinNoise(
        unsigned int seed = std::default_random_engine::default_seed) {
        p.resize(512);
        std::iota(p.begin(), p.begin() + 256, 0);

        std::default_random_engine engine(seed);
        std::ranges::shuffle(std::span(p.begin(), p.begin() + 256), engine);

        std::ranges::copy(std::span(p.begin(), p.begin() + 256),
                          p.begin() + 256);
    }

    template <std::floating_point T>
    [[nodiscard]] auto noise(T x, T y, T z) const -> T {
        // Find unit cube containing point
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        // Find relative x, y, z of point in cube
        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        // Compute fade curves for each of x, y, z
        T u = fade(x);
        T v = fade(y);
        T w = fade(z);

        // Hash coordinates of the 8 cube corners
        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        // Add blended results from 8 corners of cube
        T res = lerp(
            w,
            lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                 lerp(u, grad(p[AB], x, y - 1, z),
                      grad(p[BB], x - 1, y - 1, z))),
            lerp(v,
                 lerp(u, grad(p[AA + 1], x, y, z - 1),
                      grad(p[BA + 1], x - 1, y, z - 1)),
                 lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                      grad(p[BB + 1], x - 1, y - 1, z - 1))));
        return (res + 1) / 2;  // Normalize to [0,1]
    }

    // Octave noise function
    template <std::floating_point T>
    [[nodiscard]] auto octaveNoise(T x, T y, T z, int octaves,
                                   T persistence) const -> T {
        T total = 0;
        T frequency = 1;
        T amplitude = 1;
        T maxValue = 0;

        for (int i = 0; i < octaves; ++i) {
            total +=
                noise(x * frequency, y * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2;
        }

        return total / maxValue;
    }

    // Generate 2D noise map
    [[nodiscard]] auto generateNoiseMap(
        int width, int height, double scale, int octaves, double persistence,
        double /*lacunarity*/,
        int seed = std::default_random_engine::default_seed) const
        -> std::vector<std::vector<double>> {
        std::vector<std::vector<double>> noiseMap(height,
                                                  std::vector<double>(width));
        std::default_random_engine prng(seed);
        std::uniform_real_distribution<double> dist(-10000, 10000);
        double offsetX = dist(prng);
        double offsetY = dist(prng);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double sampleX = (x - width / 2.0 + offsetX) / scale;
                double sampleY = (y - height / 2.0 + offsetY) / scale;
                noiseMap[y][x] =
                    octaveNoise(sampleX, sampleY, 0.0, octaves, persistence);
            }
        }

        return noiseMap;
    }

private:
    std::vector<int> p;

    static constexpr auto fade(double t) noexcept -> double {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static constexpr auto lerp(double t, double a,
                               double b) noexcept -> double {
        return a + t * (b - a);
    }

    static constexpr auto grad(int hash, double x, double y,
                               double z) noexcept -> double {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};
}  // namespace atom::algorithm
#endif