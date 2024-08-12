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
    for (auto it = begin; it != end; ++it, ++i) {
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

template <typename InputIt, typename OutputIt>
void base64Decode(InputIt begin, InputIt end, OutputIt dest) {
    std::array<unsigned char, 4> charArray4{};
    std::array<unsigned char, 3> charArray3{};

    size_t i = 0;
    for (auto it = begin; it != end && *it != '='; ++it) {
        charArray4[i++] = static_cast<unsigned char>(BASE64_CHARS.find(*it));
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
}  // namespace atom::algorithm
