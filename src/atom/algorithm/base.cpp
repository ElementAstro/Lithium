/*
 * base.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: A collection of algorithms for C++

**************************************************/

#include "base.hpp"

#include <array>
#include <string_view>

#include "atom/error/exception.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#endif

#ifdef USE_SIMD
#if defined(__AVX2__) || defined(USE_AVX)
#include <immintrin.h>
#define SIMD_WIDTH 32
#elif defined(__SSE4_1__) || defined(USE_SSE)
#include <smmintrin.h>
#define SIMD_WIDTH 16
#elif defined(__ARM_NEON) || defined(USE_NEON)
#include <arm_neon.h>
#define SIMD_WIDTH 16
#endif
#endif

#if USE_OPENCL
#include <CL/cl.h>
constexpr bool HAS_OPEN_CL = true;
#else
constexpr bool HAS_OPEN_CL = false;
#endif

namespace atom::algorithm {
namespace detail {
#if USE_OPENCL
const char* base64EncodeKernelSource = R"(
        __kernel void base64EncodeKernel(__global const uchar* input, __global char* output, int size) {
            int i = get_global_id(0);
            if (i < size / 3) {
                uchar3 in = vload3(i, input);
                output[i * 4 + 0] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(in.s0 >> 2) & 0x3F];
                output[i * 4 + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((in.s0 & 0x03) << 4) | ((in.s1 >> 4) & 0x0F)];
                output[i * 4 + 2] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((in.s1 & 0x0F) << 2) | ((in.s2 >> 6) & 0x03)];
                output[i * 4 + 3] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[in.s2 & 0x3F];
            }
        }
    )";

const char* base64DecodeKernelSource = R"(
        __kernel void base64DecodeKernel(__global const char* input, __global uchar* output, int size) {
            int i = get_global_id(0);
            if (i < size / 4) {
                char4 in = vload4(i, input);
                output[i * 3 + 0] = (uchar)((in.s0 << 2) | ((in.s1 >> 4) & 0x03));
                output[i * 3 + 1] = (uchar)(((in.s1 & 0x0F) << 4) | ((in.s2 >> 2) & 0x0F));
                output[i * 3 + 2] = (uchar)(((in.s2 & 0x03) << 6) | (in.s3 & 0x3F));
            }
        }
    )";

// OpenCL kernel for XOR encryption/decryption
const char* xorKernelSource = R"(
        __kernel void xorKernel(__global const char* input, __global char* output, uchar key, int size) {
            int i = get_global_id(0);
            if (i < size) {
                output[i] = input[i] ^ key;
            }
        }
    )";

// OpenCL setup and context management
cl_context context;
cl_command_queue queue;
cl_program program;
cl_kernel base64EncodeKernel, base64DecodeKernel, xorKernel;

void initializeOpenCL() {
    // Initialize OpenCL context, compile the kernels, etc.
    // Error handling omitted for brevity
    cl_platform_id platform;
    cl_device_id device;
    clGetPlatformIDs(1, &platform, nullptr);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);
    queue = clCreateCommandQueue(context, device, 0, nullptr);

    // Compile the kernels
    const char* sources[] = {base64EncodeKernelSource, base64DecodeKernelSource,
                             xorKernelSource};
    program = clCreateProgramWithSource(context, 3, sources, nullptr, nullptr);
    clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);

    base64EncodeKernel = clCreateKernel(program, "base64EncodeKernel", nullptr);
    base64DecodeKernel = clCreateKernel(program, "base64DecodeKernel", nullptr);
    xorKernel = clCreateKernel(program, "xorKernel", nullptr);
}

void cleanupOpenCL() {
    // Cleanup OpenCL resources
    clReleaseKernel(base64EncodeKernel);
    clReleaseKernel(base64DecodeKernel);
    clReleaseKernel(xorKernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

void base64EncodeOpenCL(const unsigned char* input, char* output, size_t size) {
    cl_mem inputBuffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size,
                       (void*)input, nullptr);
    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         (size + 2) / 3 * 4, nullptr, nullptr);

    clSetKernelArg(base64EncodeKernel, 0, sizeof(cl_mem), &inputBuffer);
    clSetKernelArg(base64EncodeKernel, 1, sizeof(cl_mem), &outputBuffer);
    clSetKernelArg(base64EncodeKernel, 2, sizeof(int), &size);

    size_t globalWorkSize = (size + 2) / 3;
    clEnqueueNDRangeKernel(queue, base64EncodeKernel, 1, nullptr,
                           &globalWorkSize, nullptr, 0, nullptr, nullptr);

    clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, (size + 2) / 3 * 4,
                        output, 0, nullptr, nullptr);

    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
}

void base64DecodeOpenCL(const char* input, unsigned char* output, size_t size) {
    cl_mem inputBuffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size,
                       (void*)input, nullptr);
    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         size / 4 * 3, nullptr, nullptr);

    clSetKernelArg(base64DecodeKernel, 0, sizeof(cl_mem), &inputBuffer);
    clSetKernelArg(base64DecodeKernel, 1, sizeof(cl_mem), &outputBuffer);
    clSetKernelArg(base64DecodeKernel, 2, sizeof(int), &size);

    size_t globalWorkSize = size / 4;
    clEnqueueNDRangeKernel(queue, base64DecodeKernel, 1, nullptr,
                           &globalWorkSize, nullptr, 0, nullptr, nullptr);

    clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, size / 4 * 3, output,
                        0, nullptr, nullptr);

    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
}

void xorEncryptOpenCL(const char* input, char* output, uint8_t key,
                      size_t size) {
    cl_mem inputBuffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size,
                       (void*)input, nullptr);
    cl_mem outputBuffer =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, nullptr, nullptr);

    clSetKernelArg(xorKernel, 0, sizeof(cl_mem), &inputBuffer);
    clSetKernelArg(xorKernel, 1, sizeof(cl_mem), &outputBuffer);
    clSetKernelArg(xorKernel, 2, sizeof(uint8_t), &key);
    clSetKernelArg(xorKernel, 3, sizeof(int), &size);

    size_t globalWorkSize = size;
    clEnqueueNDRangeKernel(queue, xorKernel, 1, nullptr, &globalWorkSize,
                           nullptr, 0, nullptr, nullptr);

    clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, size, output, 0,
                        nullptr, nullptr);

    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
}
#endif
template <typename InputIt, typename OutputIt>
void base64Encode(InputIt begin, InputIt end, OutputIt dest) {
    std::array<unsigned char, 3> charArray3{};
    std::array<unsigned char, 4> charArray4{};

    size_t i = 0;
    auto it = begin;

#ifdef SIMD_AVAILABLE
    // SIMD优化部分
    constexpr size_t simdSize = 16;  // 处理16字节的输入
    std::array<unsigned char, simdSize> inputBuffer{};
    std::array<unsigned char, (simdSize / 3) * 4> outputBuffer{};

    while (std::distance(it, end) >= simdSize) {
        std::copy_n(it, simdSize, inputBuffer.begin());

#if defined(__x86_64__) || defined(_M_X64)
        // x86 SIMD实现
        __m128i input = _mm_loadu_si128(
            reinterpret_cast<const __m128i*>(inputBuffer.data()));
        __m128i mask = _mm_set1_epi32(0x3F);

        __m128i result1 = _mm_srli_epi32(input, 2);
        __m128i result2 = _mm_and_si128(_mm_slli_epi32(input, 4), mask);
        __m128i result3 = _mm_and_si128(_mm_slli_epi32(input, 2), mask);
        __m128i result4 = _mm_and_si128(input, mask);

        // 查表并存储结果
        for (int j = 0; j < 16; j += 4) {
            outputBuffer[j] = BASE64_CHARS[_mm_extract_epi8(result1, j)];
            outputBuffer[j + 1] =
                BASE64_CHARS[_mm_extract_epi8(result2, j + 1)];
            outputBuffer[j + 2] =
                BASE64_CHARS[_mm_extract_epi8(result3, j + 2)];
            outputBuffer[j + 3] =
                BASE64_CHARS[_mm_extract_epi8(result4, j + 3)];
        }
#elif defined(__ARM_NEON)
        // ARM NEON实现
        uint8x16_t input = vld1q_u8(inputBuffer.data());
        uint8x16_t mask = vdupq_n_u8(0x3F);

        uint8x16_t result1 = vshrq_n_u8(input, 2);
        uint8x16_t result2 = vandq_u8(vshlq_n_u8(input, 4), mask);
        uint8x16_t result3 = vandq_u8(vshlq_n_u8(input, 2), mask);
        uint8x16_t result4 = vandq_u8(input, mask);

        // 查表并存储结果
        for (int j = 0; j < 16; j += 4) {
            outputBuffer[j] = BASE64_CHARS[vgetq_lane_u8(result1, j)];
            outputBuffer[j + 1] = BASE64_CHARS[vgetq_lane_u8(result2, j + 1)];
            outputBuffer[j + 2] = BASE64_CHARS[vgetq_lane_u8(result3, j + 2)];
            outputBuffer[j + 3] = BASE64_CHARS[vgetq_lane_u8(result4, j + 3)];
        }
#endif

        std::copy_n(outputBuffer.begin(), (simdSize / 3) * 4, dest);
        std::advance(dest, (simdSize / 3) * 4);
        std::advance(it, simdSize);
        i += simdSize;
    }
#endif

    // 处理剩余的字节（原始实现）
    for (; it != end; ++it, ++i) {
        charArray3[i % 3] = static_cast<unsigned char>(*it);
        if (i % 3 == 2) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] =
                ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] =
                ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (int j = 0; j < 4; ++j) {
                *dest++ = BASE64_CHARS[charArray4[j]];
            }
        }
    }

    if (i % 3 != 0) {
        for (size_t j = i % 3; j < 3; ++j) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] =
            ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] =
            ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;

        for (size_t j = 0; j < i % 3 + 1; ++j) {
            *dest++ = BASE64_CHARS[charArray4[j]];
        }

        while (i++ % 3 != 0) {
            *dest++ = '=';
        }
    }
}

std::array<unsigned char, 256> createReverseLookupTable() {
    std::array<unsigned char, 256> table{};
    for (int i = 0; i < 64; ++i) {
        table[static_cast<unsigned char>(BASE64_CHARS[i])] = i;
    }
    return table;
}

const auto REVERSE_LOOKUP = createReverseLookupTable();

template <typename InputIt, typename OutputIt>
void base64Decode(InputIt begin, InputIt end, OutputIt dest) {
    std::array<unsigned char, 4> charArray4{};
    std::array<unsigned char, 3> charArray3{};

    size_t i = 0;
    auto it = begin;

#ifdef SIMD_AVAILABLE
    // SIMD优化部分
    constexpr size_t simdSize = 16;  // 处理16字节的输入
    std::array<unsigned char, simdSize> inputBuffer{};
    std::array<unsigned char, (simdSize / 4) * 3> outputBuffer{};

    while (std::distance(it, end) >= simdSize &&
           *std::next(it, simdSize - 1) != '=') {
        std::copy_n(it, simdSize, inputBuffer.begin());

#if defined(__x86_64__) || defined(_M_X64)
        // x86 SIMD实现
        __m128i input = _mm_loadu_si128(
            reinterpret_cast<const __m128i*>(inputBuffer.data()));
        __m128i lookup = _mm_setr_epi8(
            REVERSE_LOOKUP[inputBuffer[0]], REVERSE_LOOKUP[inputBuffer[1]],
            REVERSE_LOOKUP[inputBuffer[2]], REVERSE_LOOKUP[inputBuffer[3]],
            REVERSE_LOOKUP[inputBuffer[4]], REVERSE_LOOKUP[inputBuffer[5]],
            REVERSE_LOOKUP[inputBuffer[6]], REVERSE_LOOKUP[inputBuffer[7]],
            REVERSE_LOOKUP[inputBuffer[8]], REVERSE_LOOKUP[inputBuffer[9]],
            REVERSE_LOOKUP[inputBuffer[10]], REVERSE_LOOKUP[inputBuffer[11]],
            REVERSE_LOOKUP[inputBuffer[12]], REVERSE_LOOKUP[inputBuffer[13]],
            REVERSE_LOOKUP[inputBuffer[14]], REVERSE_LOOKUP[inputBuffer[15]]);

        __m128i merged = _mm_or_si128(
            _mm_or_si128(_mm_slli_epi32(lookup, 18),
                         _mm_slli_epi32(_mm_and_si128(_mm_srli_epi32(lookup, 8),
                                                      _mm_set1_epi32(0x3F)),
                                        12)),
            _mm_or_si128(
                _mm_slli_epi32(_mm_and_si128(_mm_srli_epi32(lookup, 16),
                                             _mm_set1_epi32(0x3F)),
                               6),
                _mm_and_si128(_mm_srli_epi32(lookup, 24),
                              _mm_set1_epi32(0x3F))));

        __m128i result =
            _mm_shuffle_epi8(merged, _mm_setr_epi8(2, 1, 0, 6, 5, 4, 10, 9, 8,
                                                   14, 13, 12, -1, -1, -1, -1));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(outputBuffer.data()),
                         result);

#elif defined(__ARM_NEON)
        // ARM NEON实现
        uint8x16_t input = vld1q_u8(inputBuffer.data());
        uint8x16_t lookup = vcreate_u8(0);
        for (int j = 0; j < 16; ++j) {
            lookup = vsetq_lane_u8(REVERSE_LOOKUP[inputBuffer[j]], lookup, j);
        }

        uint32x4_t merged = vorrq_u32(
            vorrq_u32(
                vshlq_n_u32(vreinterpretq_u32_u8(lookup), 18),
                vshlq_n_u32(
                    vandq_u32(vshrq_n_u32(vreinterpretq_u32_u8(lookup), 8),
                              vdupq_n_u32(0x3F)),
                    12)),
            vorrq_u32(
                vshlq_n_u32(
                    vandq_u32(vshrq_n_u32(vreinterpretq_u32_u8(lookup), 16),
                              vdupq_n_u32(0x3F)),
                    6),
                vandq_u32(vshrq_n_u32(vreinterpretq_u32_u8(lookup), 24),
                          vdupq_n_u32(0x3F))));

        uint8x16_t result = vqtbl1q_u8(vreinterpretq_u8_u32(merged),
                                       vld1q_u8({2, 1, 0, 6, 5, 4, 10, 9, 8, 14,
                                                 13, 12, 255, 255, 255, 255}));
        vst1q_u8(outputBuffer.data(), result);
#endif

        std::copy_n(outputBuffer.begin(), (simdSize / 4) * 3, dest);
        std::advance(dest, (simdSize / 4) * 3);
        std::advance(it, simdSize);
    }
#endif

    for (; it != end && *it != '='; ++it) {
        charArray4[i++] = REVERSE_LOOKUP[static_cast<unsigned char>(*it)];
        if (i == 4) {
            charArray3[0] =
                (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] =
                ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; i < 3; ++i) {
                *dest++ = charArray3[i];
            }
            i = 0;
        }
    }

    if (i != 0) {
        for (size_t j = i; j < 4; ++j) {
            charArray4[j] = 0;
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] =
            ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);

        for (size_t j = 0; j < i - 1; ++j) {
            *dest++ = charArray3[j];
        }
    }
}
}  // namespace detail

auto base64Encode(std::string_view bytes_to_encode) -> std::string {
    std::string ret;
    ret.reserve((bytes_to_encode.size() + 2) / 3 * 4);

    if (HAS_OPEN_CL) {
#if USE_OPENCL
        detail::base64EncodeOpenCL(
            reinterpret_cast<const unsigned char*>(bytes_to_encode.data()),
            ret.data(), bytes_to_encode.size());
#endif
    } else {
        detail::base64Encode(bytes_to_encode.begin(), bytes_to_encode.end(),
                             std::back_inserter(ret));
    }

    return ret;
}

auto base64Decode(std::string_view encoded_string) -> std::string {
    std::string ret;
    ret.reserve(encoded_string.size() / 4 * 3);

    if (HAS_OPEN_CL) {
#if USE_OPENCL
        detail::base64DecodeOpenCL(encoded_string.data(),
                                   reinterpret_cast<unsigned char*>(ret.data()),
                                   encoded_string.size());
#endif
    } else {
        detail::base64Decode(encoded_string.begin(), encoded_string.end(),
                             std::back_inserter(ret));
    }

    return ret;
}

auto fbase64Encode(std::span<const unsigned char> input) -> std::string {
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);

    if (HAS_OPEN_CL) {
#if USE_OPENCL
        detail::base64EncodeOpenCL(input.data(), output.data(), input.size());
#endif
    } else {
        detail::base64Encode(input.begin(), input.end(),
                             std::back_inserter(output));
    }

    return output;
}

auto fbase64Decode(std::span<const char> input) -> std::vector<unsigned char> {
    if (input.size() % 4 != 0) {
        THROW_INVALID_ARGUMENT("Invalid base64 input length");
    }

    std::vector<unsigned char> output;
    output.reserve(input.size() / 4 * 3);

    if (HAS_OPEN_CL) {
#if USE_OPENCL
        detail::base64DecodeOpenCL(input.data(), output.data(), input.size());
#endif
    } else {
        detail::base64Decode(input.begin(), input.end(),
                             std::back_inserter(output));
    }

    return output;
}

auto xorEncrypt(std::string_view plaintext, uint8_t key) -> std::string {
    std::string ciphertext;
    ciphertext.reserve(plaintext.size());

    if (HAS_OPEN_CL) {
#if USE_OPENCL
        detail::xorEncryptOpenCL(plaintext.data(), ciphertext.data(), key,
                                 plaintext.size());
#endif
    } else {
        for (char c : plaintext) {
            ciphertext.push_back(
                static_cast<char>(static_cast<uint8_t>(c) ^ key));
        }
    }

    return ciphertext;
}

auto xorDecrypt(std::string_view ciphertext, uint8_t key) -> std::string {
    return xorEncrypt(ciphertext, key);
}

constexpr std::string_view BASE32_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
constexpr int BITS_PER_BYTE = 8;
constexpr int BITS_PER_BASE32_CHAR = 5;
constexpr uint32_t BASE32_MASK = 0x1F;
constexpr uint32_t BYTE_MASK = 0xFF;

#ifdef USE_MP
#pragma omp parallel for
#endif
auto encodeBase32(const std::vector<uint8_t>& data) -> std::string {
    std::string encoded;
    size_t bitCount = 0;
    uint32_t buffer = 0;

#ifdef USE_SIMD
    size_t simdChunkSize = SIMD_WIDTH / BITS_PER_BYTE;  // 每个SIMD块的字节数

    for (size_t i = 0; i + simdChunkSize <= data.size(); i += simdChunkSize) {
#if defined(USE_AVX) || defined(__AVX2__)
        __m256i simdData =
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&data[i]));
        uint32_t simdVal = _mm256_extract_epi32(simdData, 0);  // 提取低32位
#elif defined(USE_SSE) || defined(__SSE4_1__)
        __m128i simdData =
            _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i]));
        uint32_t simdVal = _mm_extract_epi32(simdData, 0);  // 提取低32位
#elif defined(USE_NEON) || defined(__ARM_NEON)
        uint8x16_t simdData = vld1q_u8(&data[i]);
        uint32_t simdVal = vgetq_lane_u32(vreinterpretq_u32_u8(simdData), 0);
#endif

        for (int j = 0; j < 5; ++j) {
            uint8_t index =
                (simdVal >> (27 - j * BITS_PER_BASE32_CHAR)) & BASE32_MASK;
            encoded += BASE32_ALPHABET[index];
        }
    }

    for (size_t i = (data.size() / simdChunkSize) * simdChunkSize;
         i < data.size(); ++i) {
        buffer = (buffer << BITS_PER_BYTE) | data[i];
        bitCount += BITS_PER_BYTE;
        while (bitCount >= BITS_PER_BASE32_CHAR) {
            bitCount -= BITS_PER_BASE32_CHAR;
            encoded += BASE32_ALPHABET[(buffer >> bitCount) & BASE32_MASK];
        }
    }
#else
    // 非SIMD编码流程
#ifdef USE_MP
#pragma omp parallel for
#endif
    for (uint8_t byte : data) {
        buffer = (buffer << BITS_PER_BYTE) | byte;
        bitCount += BITS_PER_BYTE;
        while (bitCount >= BITS_PER_BASE32_CHAR) {
            bitCount -= BITS_PER_BASE32_CHAR;
            encoded += BASE32_ALPHABET[(buffer >> bitCount) & BASE32_MASK];
        }
    }
#endif

    if (bitCount > 0) {
        encoded +=
            BASE32_ALPHABET[(buffer << (BITS_PER_BASE32_CHAR - bitCount)) &
                            BASE32_MASK];
    }

    while (encoded.size() % 8 != 0) {
        encoded += '=';
    }

    return encoded;
}

// 解码函数
#ifdef USE_MP
#pragma omp parallel for
#endif
auto decodeBase32(const std::string& encoded) -> std::vector<uint8_t> {
    std::vector<uint8_t> decoded;
    size_t bitCount = 0;
    uint32_t buffer = 0;

#ifdef USE_SIMD
    size_t simdChunkSize = SIMD_WIDTH / BITS_PER_BYTE;

    for (size_t i = 0; i + simdChunkSize <= encoded.size();
         i += simdChunkSize) {
#if defined(USE_AVX) || defined(__AVX2__)
        __m256i simdEncoded =
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&encoded[i]));
#elif defined(USE_SSE) || defined(__SSE4_1__)
        __m128i simdEncoded =
            _mm_loadu_si128(reinterpret_cast<const __m128i*>(&encoded[i]));
#elif defined(USE_NEON) || defined(__ARM_NEON)
        uint8x16_t simdEncoded =
            vld1q_u8(reinterpret_cast<const uint8_t*>(&encoded[i]));
#endif

        for (int j = 0; j < simdChunkSize; ++j) {
            int idx = BASE32_ALPHABET.find(encoded[i + j]);
            if (idx == std::string::npos) {
                throw std::invalid_argument("无效字符在Base32编码中");
            }
            buffer = (buffer << BITS_PER_BASE32_CHAR) | idx;
            bitCount += BITS_PER_BASE32_CHAR;
            if (bitCount >= BITS_PER_BYTE) {
                bitCount -= BITS_PER_BYTE;
                decoded.push_back(
                    static_cast<uint8_t>((buffer >> bitCount) & BYTE_MASK));
            }
        }
    }
#else
    for (char character : encoded) {
        if (character == '=') {
            break;
        }
        auto index = BASE32_ALPHABET.find(character);
        if (index == std::string::npos) {
            THROW_INVALID_ARGUMENT("Invalid character in Base32 encoding");
        }

        buffer = (buffer << BITS_PER_BASE32_CHAR) | index;
        bitCount += BITS_PER_BASE32_CHAR;
        if (bitCount >= BITS_PER_BYTE) {
            bitCount -= BITS_PER_BYTE;
            decoded.push_back(
                static_cast<uint8_t>((buffer >> bitCount) & BYTE_MASK));
        }
    }
#endif

    return decoded;
}

#ifdef USE_CL
// 读取OpenCL内核文件
auto readKernelSource(const std::string& filename) -> std::string {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开内核文件");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 使用OpenCL进行Base32编码
auto encodeBase32CL(const std::vector<uint8_t>& data) -> std::string {
    // OpenCL平台和设备初始化
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty()) {
        throw std::runtime_error("没有可用的OpenCL平台");
    }

    // 选择第一个平台和设备
    cl::Platform platform = platforms[0];
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (devices.empty()) {
        throw std::runtime_error("没有可用的GPU设备");
    }
    cl::Device device = devices[0];

    // 创建OpenCL上下文和命令队列
    cl::Context context(device);
    cl::CommandQueue queue(context, device);

    // 读取内核源代码
    std::string kernelSource = readKernelSource("base32_encode_kernel.cl");
    cl::Program::Sources sources(1, std::make_pair(kernelSource.c_str(), kernelSource.size()));

    // 构建程序
    cl::Program program(context, sources);
    if (program.build({device}) != CL_SUCCESS) {
        throw std::runtime_error("内核程序构建失败");
    }

    // 分配输入和输出缓冲区
    size_t dataSize = data.size();
    size_t encodedSize = ((dataSize * 8) + 4) / 5;  // Base32输出大小

    cl::Buffer inputBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, dataSize, (void*)data.data());
    cl::Buffer outputBuffer(context, CL_MEM_WRITE_ONLY, encodedSize);

    // 设置内核参数
    cl::Kernel kernel(program, "base32_encode");
    kernel.setArg(0, inputBuffer);
    kernel.setArg(1, outputBuffer);
    kernel.setArg(2, static_cast<uint32_t>(dataSize));

    // 执行内核
    cl::NDRange global(dataSize);  // 数据大小定义全局工作量
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    queue.finish();

    // 读取结果
    std::vector<char> encoded(encodedSize);
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, encodedSize, encoded.data());

    // 将编码结果转成字符串
    return std::string(encoded.begin(), encoded.end());
}
#endif
}  // namespace atom::algorithm
