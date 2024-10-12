#ifndef ATOM_ALGORITHM_PERLIN_HPP
#define ATOM_ALGORITHM_PERLIN_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <numeric>
#include <random>
#include <span>
#include <vector>

#ifdef USE_OPENCL  // 宏定义：是否启用OpenCL
#include <CL/cl.h>
#endif

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

#ifdef USE_OPENCL
        initializeOpenCL();
#endif
    }

    ~PerlinNoise() {
#ifdef USE_OPENCL
        cleanupOpenCL();
#endif
    }

    template <std::floating_point T>
    [[nodiscard]] auto noise(T x, T y, T z) const -> T {
#ifdef USE_OPENCL
        if (opencl_available_) {
            return noiseOpenCL(x, y, z);
        }
#endif
        return noiseCPU(x, y, z);
    }

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

#ifdef USE_OPENCL
    cl_context context_;
    cl_command_queue queue_;
    cl_program program_;
    cl_kernel noise_kernel_;
    bool opencl_available_;

    void initializeOpenCL() {
        cl_int err;
        cl_platform_id platform;
        cl_device_id device;

        err = clGetPlatformIDs(1, &platform, nullptr);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        context_ = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        queue_ = clCreateCommandQueue(context_, device, 0, &err);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        const char* kernel_source = R"CLC(
            __kernel void noise_kernel(__global const float* coords,
                                       __global float* result,
                                       __constant int* p) {
                int gid = get_global_id(0);

                float x = coords[gid * 3];
                float y = coords[gid * 3 + 1];
                float z = coords[gid * 3 + 2];

                int X = ((int)floor(x)) & 255;
                int Y = ((int)floor(y)) & 255;
                int Z = ((int)floor(z)) & 255;

                x -= floor(x);
                y -= floor(y);
                z -= floor(z);

                float u = x * x * x * (x * (x * 6 - 15) + 10);
                float v = y * y * y * (y * (y * 6 - 15) + 10);
                float w = z * z * z * (z * (z * 6 - 15) + 10);

                int A = p[X] + Y;
                int AA = p[A] + Z;
                int AB = p[A + 1] + Z;
                int B = p[X + 1] + Y;
                int BA = p[B] + Z;
                int BB = p[B + 1] + Z;

                float res = lerp(w,
                                 lerp(v, lerp(u, grad(p[AA], x, y, z),
                                              grad(p[BA], x - 1, y, z)),
                                      lerp(u, grad(p[AB], x, y - 1, z),
                                           grad(p[BB], x - 1, y - 1, z))),
                                 lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                              grad(p[BA + 1], x - 1, y, z - 1)),
                                      lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                           grad(p[BB + 1], x - 1, y - 1,
                                                z - 1))));
                result[gid] = (res + 1) / 2;
            }

            float lerp(float t, float a, float b) {
                return a + t * (b - a);
            }

            float grad(int hash, float x, float y, float z) {
                int h = hash & 15;
                float u = h < 8 ? x : y;
                float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
                return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
            }
        )CLC";

        program_ = clCreateProgramWithSource(context_, 1, &kernel_source,
                                             nullptr, &err);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        err = clBuildProgram(program_, 1, &device, nullptr, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        noise_kernel_ = clCreateKernel(program_, "noise_kernel", &err);
        if (err != CL_SUCCESS) {
            opencl_available_ = false;
            return;
        }

        opencl_available_ = true;
    }

    void cleanupOpenCL() {
        if (opencl_available_) {
            clReleaseKernel(noise_kernel_);
            clReleaseProgram(program_);
            clReleaseCommandQueue(queue_);
            clReleaseContext(context_);
        }
    }

    template <std::floating_point T>
    auto noiseOpenCL(T x, T y, T z) const -> T {
        float coords[] = {static_cast<float>(x), static_cast<float>(y),
                          static_cast<float>(z)};
        float result;

        cl_mem coords_buffer =
            clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           sizeof(coords), coords, nullptr);
        cl_mem result_buffer = clCreateBuffer(context_, CL_MEM_WRITE_ONLY,
                                              sizeof(float), nullptr, nullptr);
        cl_mem p_buffer =
            clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           p.size() * sizeof(int), p.data(), nullptr);

        clSetKernelArg(noise_kernel_, 0, sizeof(cl_mem), &coords_buffer);
        clSetKernelArg(noise_kernel_, 1, sizeof(cl_mem), &result_buffer);
        clSetKernelArg(noise_kernel_, 2, sizeof(cl_mem), &p_buffer);

        size_t global_work_size = 1;
        clEnqueueNDRangeKernel(queue_, noise_kernel_, 1, nullptr,
                               &global_work_size, nullptr, 0, nullptr, nullptr);

        clEnqueueReadBuffer(queue_, result_buffer, CL_TRUE, 0, sizeof(float),
                            &result, 0, nullptr, nullptr);

        clReleaseMemObject(coords_buffer);
        clReleaseMemObject(result_buffer);
        clReleaseMemObject(p_buffer);

        return static_cast<T>(result);
    }
#endif  // USE_OPENCL

    template <std::floating_point T>
    [[nodiscard]] auto noiseCPU(T x, T y, T z) const -> T {
        // Find unit cube containing point
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        // Find relative x, y, z of point in cube
        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

// Compute fade curves for each of x, y, z
#ifdef USE_SIMD
        // SIMD-based fade function calculations
        __m256d xSimd = _mm256_set1_pd(x);
        __m256d ySimd = _mm256_set1_pd(y);
        __m256d zSimd = _mm256_set1_pd(z);

        __m256d uSimd =
            _mm256_mul_pd(xSimd, _mm256_sub_pd(xSimd, _mm256_set1_pd(15)));
        uSimd = _mm256_mul_pd(
            uSimd, _mm256_add_pd(_mm256_set1_pd(10),
                                 _mm256_mul_pd(xSimd, _mm256_set1_pd(6))));
        // Apply similar SIMD operations for v and w if needed
        __m256d vSimd =
            _mm256_mul_pd(ySimd, _mm256_sub_pd(ySimd, _mm256_set1_pd(15)));
        vSimd = _mm256_mul_pd(
            vSimd, _mm256_add_pd(_mm256_set1_pd(10),
                                 _mm256_mul_pd(ySimd, _mm256_set1_pd(6))));
        __m256d wSimd =
            _mm256_mul_pd(zSimd, _mm256_sub_pd(zSimd, _mm256_set1_pd(15)));
        wSimd = _mm256_mul_pd(
            wSimd, _mm256_add_pd(_mm256_set1_pd(10),
                                 _mm256_mul_pd(zSimd, _mm256_set1_pd(6))));
#else
        T u = fade(x);
        T v = fade(y);
        T w = fade(z);
#endif

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

#endif  // ATOM_ALGORITHM_PERLIN_HPP
