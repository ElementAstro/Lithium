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
// Keccak state constants
constexpr size_t K_KECCAK_F_RATE = 1088;  // For Keccak-256
constexpr size_t K_ROUNDS = 24;
constexpr size_t K_STATE_SIZE = 5;
constexpr size_t K_RATE_IN_BYTES = K_KECCAK_F_RATE / 8;
constexpr uint8_t K_PADDING_BYTE = 0x06;
constexpr uint8_t K_PADDING_LAST_BYTE = 0x80;

// Round constants for Keccak
constexpr std::array<uint64_t, K_ROUNDS> K_ROUND_CONSTANTS = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

// Rotation offsets
constexpr std::array<std::array<size_t, K_STATE_SIZE>, K_STATE_SIZE>
    K_ROTATION_CONSTANTS = {{{0, 1, 62, 28, 27},
                             {36, 44, 6, 55, 20},
                             {3, 10, 43, 25, 39},
                             {41, 45, 15, 21, 8},
                             {18, 2, 61, 56, 14}}};

// Keccak state as 5x5 matrix of 64-bit integers
using StateArray = std::array<std::array<uint64_t, K_STATE_SIZE>, K_STATE_SIZE>;

namespace {
#if USE_OPENCL
const char *minhashKernelSource = R"CLC(
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

auto MinHash::jaccardIndex(const std::vector<size_t> &sig1,
                           const std::vector<size_t> &sig2) -> double {
    size_t equalCount = 0;

    for (size_t i = 0; i < sig1.size(); ++i) {
        if (sig1[i] == sig2[i]) {
            ++equalCount;
        }
    }

    return static_cast<double>(equalCount) / sig1.size();
}

auto hexstringFromData(const std::string &data) -> std::string {
    const char *hexChars = "0123456789ABCDEF";
    std::string output;
    output.reserve(data.size() * 2);  // Reserve space for the hex string

    for (unsigned char byte : data) {
        output.push_back(hexChars[(byte >> 4) & 0x0F]);
        output.push_back(hexChars[byte & 0x0F]);
    }

    return output;
}

auto dataFromHexstring(const std::string &data) -> std::string {
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

// θ step: XOR each column and then propagate changes across the state
inline void theta(StateArray &stateArray) {
    std::array<uint64_t, K_STATE_SIZE> column, diff;
    for (size_t colIndex = 0; colIndex < K_STATE_SIZE; ++colIndex) {
        column[colIndex] = stateArray[colIndex][0] ^ stateArray[colIndex][1] ^
                           stateArray[colIndex][2] ^ stateArray[colIndex][3] ^
                           stateArray[colIndex][4];
    }
    for (size_t colIndex = 0; colIndex < K_STATE_SIZE; ++colIndex) {
        diff[colIndex] = column[(colIndex + 4) % K_STATE_SIZE] ^
                         std::rotl(column[(colIndex + 1) % K_STATE_SIZE], 1);
        for (size_t rowIndex = 0; rowIndex < K_STATE_SIZE; ++rowIndex) {
            stateArray[colIndex][rowIndex] ^= diff[colIndex];
        }
    }
}

// ρ step: Rotate each bit-plane by pre-determined offsets
inline void rho(StateArray &stateArray) {
    for (size_t colIndex = 0; colIndex < K_STATE_SIZE; ++colIndex) {
        for (size_t rowIndex = 0; rowIndex < K_STATE_SIZE; ++rowIndex) {
            stateArray[colIndex][rowIndex] = std::rotl(
                stateArray[colIndex][rowIndex],
                static_cast<int>(K_ROTATION_CONSTANTS[colIndex][rowIndex]));
        }
    }
}

// π step: Permute bits to new positions based on a fixed pattern
inline void pi(StateArray &stateArray) {
    StateArray temp = stateArray;
    for (size_t colIndex = 0; colIndex < K_STATE_SIZE; ++colIndex) {
        for (size_t rowIndex = 0; rowIndex < K_STATE_SIZE; ++rowIndex) {
            stateArray[colIndex][rowIndex] =
                temp[(colIndex + 3 * rowIndex) % K_STATE_SIZE][colIndex];
        }
    }
}

// χ step: Non-linear step XORs data across rows, producing diffusion
inline void chi(StateArray &stateArray) {
    for (size_t rowIndex = 0; rowIndex < K_STATE_SIZE; ++rowIndex) {
        std::array<uint64_t, K_STATE_SIZE> temp = stateArray[rowIndex];
        for (size_t colIndex = 0; colIndex < K_STATE_SIZE; ++colIndex) {
            stateArray[colIndex][rowIndex] ^=
                (~temp[(colIndex + 1) % K_STATE_SIZE] &
                 temp[(colIndex + 2) % K_STATE_SIZE]);
        }
    }
}

// ι step: XOR a round constant into the first state element
inline void iota(StateArray &stateArray, size_t round) {
    stateArray[0][0] ^= K_ROUND_CONSTANTS[round];
}

// Keccak-p permutation: 24 rounds of transformations on the state
inline void keccakP(StateArray &stateArray) {
    for (size_t round = 0; round < K_ROUNDS; ++round) {
        theta(stateArray);
        rho(stateArray);
        pi(stateArray);
        chi(stateArray);
        iota(stateArray, round);
    }
}

// Absorb phase: XOR input into the state and permute
void absorb(StateArray &state, const uint8_t *input, size_t length) {
    while (length >= K_RATE_IN_BYTES) {
        for (size_t i = 0; i < K_RATE_IN_BYTES / 8; ++i) {
            state[i % K_STATE_SIZE][i / K_STATE_SIZE] ^=
                std::bit_cast<uint64_t>(input + i * 8);
        }
        keccakP(state);
        input += K_RATE_IN_BYTES;
        length -= K_RATE_IN_BYTES;
    }
}

// Padding and absorbing the last block
void padAndAbsorb(StateArray &state, const uint8_t *input, size_t length) {
    std::array<uint8_t, K_RATE_IN_BYTES> paddedBlock = {};
    std::memcpy(paddedBlock.data(), input, length);
    paddedBlock[length] = K_PADDING_BYTE;       // Keccak padding
    paddedBlock.back() |= K_PADDING_LAST_BYTE;  // Set last bit to 1
    absorb(state, paddedBlock.data(), paddedBlock.size());
}

// Squeeze phase: Extract output from the state
void squeeze(StateArray &state, uint8_t *output, size_t outputLength) {
    while (outputLength >= K_RATE_IN_BYTES) {
        for (size_t i = 0; i < K_RATE_IN_BYTES / 8; ++i) {
            std::memcpy(output + i * 8,
                        &state[i % K_STATE_SIZE][i / K_STATE_SIZE], 8);
        }
        keccakP(state);
        output += K_RATE_IN_BYTES;
        outputLength -= K_RATE_IN_BYTES;
    }
    for (size_t i = 0; i < outputLength / 8; ++i) {
        std::memcpy(output + i * 8, &state[i % K_STATE_SIZE][i / K_STATE_SIZE],
                    8);
    }
}

// Keccak-256 hashing function
auto keccak256(const uint8_t *input,
               size_t length) -> std::array<uint8_t, K_HASH_SIZE> {
    StateArray state = {};
    padAndAbsorb(state, input, length);

    std::array<uint8_t, K_HASH_SIZE> hash = {};
    squeeze(state, hash.data(), hash.size());
    return hash;
}

}  // namespace atom::algorithm
