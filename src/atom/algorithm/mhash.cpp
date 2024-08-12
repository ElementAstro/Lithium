/*
 * mhash.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Implementation of murmur3 hash and quick hash

**************************************************/

#include "mhash.hpp"

#include <charconv>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>

#include "atom/error/exception.hpp"
#include "atom/utils/random.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace atom::algorithm {
namespace {
#if USE_OPENCL
const char* minhashKernelSource = R"CLC(
__kernel void minhash_kernel(__global const size_t* hashes, __global size_t* signature, __global const size_t* a_values, __global const size_t* b_values, const size_t p, const size_t num_hashes, const size_t num_elements) {
    int gid = get_global_id(0);
    if (gid < num_hashes) {
        size_t min_hash = SIZE_MAX;
        size_t a = a_values[gid];
        size_t b = b_values[gid];
        for (size_t i = 0; i < num_elements; ++i) {
            size_t h = (a * hashes[i] + b) % p;
            if (h < min_hash) {
                min_hash = h;
            }
        }
        signature[gid] = min_hash;
    }
}
)CLC";
#endif
}  // anonymous namespace

MinHash::MinHash(size_t num_hashes)
#if USE_OPENCL
    : opencl_available_(false)
#endif
{
    hash_functions_.reserve(num_hashes);
    for (size_t i = 0; i < num_hashes; ++i) {
        hash_functions_.emplace_back(generateHashFunction());
    }
#if USE_OPENCL
    initializeOpenCL();
#endif
}

MinHash::~MinHash() {
#if USE_OPENCL
    cleanupOpenCL();
#endif
}

#if USE_OPENCL
void MinHash::initializeOpenCL() {
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;

    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err != CL_SUCCESS) {
        return;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    if (err != CL_SUCCESS) {
        return;
    }

    context_ = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        return;
    }

    queue_ = clCreateCommandQueue(context_, device, 0, &err);
    if (err != CL_SUCCESS) {
        return;
    }

    program_ = clCreateProgramWithSource(context_, 1, &minhashKernelSource,
                                         nullptr, &err);
    if (err != CL_SUCCESS) {
        return;
    }

    err = clBuildProgram(program_, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        return;
    }

    minhash_kernel_ = clCreateKernel(program_, "minhash_kernel", &err);
    if (err == CL_SUCCESS) {
        opencl_available_ = true;
    }
}

void MinHash::cleanupOpenCL() {
    if (opencl_available_) {
        clReleaseKernel(minhash_kernel_);
        clReleaseProgram(program_);
        clReleaseCommandQueue(queue_);
        clReleaseContext(context_);
    }
}
#endif

auto MinHash::generateHashFunction() -> HashFunction {
    utils::Random<std::mt19937, std::uniform_int_distribution<>> rand(
        0, std::numeric_limits<int>::max());

    size_t a = rand();
    size_t b = rand();
    size_t p = std::numeric_limits<size_t>::max();

    return [a, b, p](size_t x) -> size_t { return (a * x + b) % p; };
}

auto MinHash::jaccardIndex(const std::vector<size_t>& sig1,
                           const std::vector<size_t>& sig2) -> double {
    size_t equalCount = 0;

    for (size_t i = 0; i < sig1.size(); ++i) {
        if (sig1[i] == sig2[i]) {
            ++equalCount;
        }
    }

    return static_cast<double>(equalCount) / sig1.size();
}

auto hexstringFromData(const std::string& data) -> std::string {
    const char* hexChars = "0123456789ABCDEF";
    std::string output;
    output.reserve(data.size() * 2);  // Reserve space for the hex string

    for (unsigned char byte : data) {
        output.push_back(hexChars[(byte >> 4) & 0x0F]);
        output.push_back(hexChars[byte & 0x0F]);
    }

    return output;
}

auto dataFromHexstring(const std::string& data) -> std::string {
    if (data.size() % 2 != 0) {
        THROW_INVALID_ARGUMENT("Hex string length must be even");
    }

    std::string result;
    result.resize(data.size() / 2);

    size_t outputIndex = 0;
    for (size_t i = 0; i < data.size(); i += 2) {
        int byte = 0;
        auto [ptr, ec] =
            std::from_chars(data.data() + i, data.data() + i + 2, byte, 16);

        if (ec == std::errc::invalid_argument || ptr != data.data() + i + 2) {
            THROW_INVALID_ARGUMENT("Invalid hex character");
        }

        result[outputIndex++] = static_cast<char>(byte);
    }

    return result;
}

}  // namespace atom::algorithm
