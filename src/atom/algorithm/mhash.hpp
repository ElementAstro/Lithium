/*
 * mhash.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Implementation of murmur3 hash and quick hash

**************************************************/

#ifndef ATOM_ALGORITHM_MHASH_HPP
#define ATOM_ALGORITHM_MHASH_HPP

#include <functional>
#include <limits>
#include <ranges>
#include <string>
#include <vector>

#if USE_OPENCL
#include <CL/cl.h>
#endif

#include "macro.hpp"

namespace atom::algorithm {
/**
 * @brief Converts a string to a hexadecimal string representation.
 *
 * @param data The input string.
 * @return std::string The hexadecimal string representation.
 */
ATOM_NODISCARD auto hexstringFromData(const std::string& data) -> std::string;

/**
 * @brief Converts a hexadecimal string representation to binary data.
 *
 * @param data The input hexadecimal string.
 * @return std::string The binary data.
 * @throw std::invalid_argument If the input hexstring is not a valid
 * hexadecimal string.
 */
ATOM_NODISCARD auto dataFromHexstring(const std::string& data) -> std::string;

/**
 * @brief Implements the MinHash algorithm for estimating Jaccard similarity.
 *
 * The MinHash algorithm generates hash signatures for sets and estimates the
 * Jaccard index between sets based on these signatures.
 */
class MinHash {
public:
    /**
     * @brief Type definition for a hash function used in MinHash.
     */
    using HashFunction = std::function<size_t(size_t)>;

    /**
     * @brief Constructs a MinHash object with a specified number of hash
     * functions.
     *
     * @param num_hashes The number of hash functions to use for MinHash.
     */
    explicit MinHash(size_t num_hashes);

    /**
     * @brief Destructor to clean up OpenCL resources.
     */
    ~MinHash();

    /**
     * @brief Computes the MinHash signature (hash values) for a given set.
     *
     * @tparam Range Type of the range representing the set elements.
     * @param set The set for which to compute the MinHash signature.
     * @return std::vector<size_t> MinHash signature (hash values) for the set.
     */
    template <std::ranges::range Range>
    auto computeSignature(const Range& set) const -> std::vector<size_t> {
        std::vector<size_t> signature(hash_functions_.size(),
                                      std::numeric_limits<size_t>::max());
#if USE_OPENCL
        if (opencl_available_) {
            computeSignatureOpenCL(set, signature);
        } else {
#endif
            for (const auto& element : set) {
                size_t elementHash =
                    std::hash<typename Range::value_type>{}(element);
                for (size_t i = 0; i < hash_functions_.size(); ++i) {
                    signature[i] =
                        std::min(signature[i], hash_functions_[i](elementHash));
                }
            }
#if USE_OPENCL
        }
#endif
        return signature;
    }

    /**
     * @brief Computes the Jaccard index between two sets based on their MinHash
     * signatures.
     *
     * @param sig1 MinHash signature of the first set.
     * @param sig2 MinHash signature of the second set.
     * @return double Estimated Jaccard index between the two sets.
     */
    static auto jaccardIndex(const std::vector<size_t>& sig1,
                             const std::vector<size_t>& sig2) -> double;

private:
    /**
     * @brief Vector of hash functions used for MinHash.
     */
    std::vector<HashFunction> hash_functions_;

    /**
     * @brief Generates a hash function suitable for MinHash.
     *
     * @return HashFunction Generated hash function.
     */
    static auto generateHashFunction() -> HashFunction;

#if USE_OPENCL
    /**
     * @brief OpenCL resources and state.
     */
    cl_context context_;
    cl_command_queue queue_;
    cl_program program_;
    cl_kernel minhash_kernel_;
    bool opencl_available_;

    /**
     * @brief Initializes OpenCL context and resources.
     */
    void initializeOpenCL();

    /**
     * @brief Cleans up OpenCL resources.
     */
    void cleanupOpenCL();

    /**
     * @brief Computes the MinHash signature using OpenCL.
     *
     * @tparam Range Type of the range representing the set elements.
     * @param set The set for which to compute the MinHash signature.
     * @param signature The vector to store the computed signature.
     */
    template <std::ranges::range Range>
    void computeSignatureOpenCL(const Range& set,
                                std::vector<size_t>& signature) const {
        cl_int err;
        size_t numHashes = hash_functions_.size();
        size_t numElements = set.size();

        std::vector<size_t> hashes;
        hashes.reserve(numElements);
        for (const auto& element : set) {
            hashes.push_back(std::hash<typename Range::value_type>{}(element));
        }

        std::vector<size_t> aValues(numHashes);
        std::vector<size_t> bValues(numHashes);
        for (size_t i = 0; i < numHashes; ++i) {
            aValues;  // Use the generated hash function's "a" value
            bValues;  // Use the generated hash function's "b" value
        }

        cl_mem hashesBuffer =
            clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           numElements * sizeof(size_t), hashes.data(), &err);
        cl_mem signatureBuffer =
            clCreateBuffer(context_, CL_MEM_WRITE_ONLY,
                           numHashes * sizeof(size_t), nullptr, &err);
        cl_mem aValuesBuffer =
            clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           numHashes * sizeof(size_t), aValues.data(), &err);
        cl_mem bValuesBuffer =
            clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           numHashes * sizeof(size_t), bValues.data(), &err);

        size_t p = std::numeric_limits<size_t>::max();

        clSetKernelArg(minhash_kernel_, 0, sizeof(cl_mem), &hashesBuffer);
        clSetKernelArg(minhash_kernel_, 1, sizeof(cl_mem), &signatureBuffer);
        clSetKernelArg(minhash_kernel_, 2, sizeof(cl_mem), &aValuesBuffer);
        clSetKernelArg(minhash_kernel_, 3, sizeof(cl_mem), &bValuesBuffer);
        clSetKernelArg(minhash_kernel_, 4, sizeof(size_t), &p);
        clSetKernelArg(minhash_kernel_, 5, sizeof(size_t), &numHashes);
        clSetKernelArg(minhash_kernel_, 6, sizeof(size_t), &numElements);

        size_t globalWorkSize = numHashes;
        clEnqueueNDRangeKernel(queue_, minhash_kernel_, 1, nullptr,
                               &globalWorkSize, nullptr, 0, nullptr, nullptr);

        clEnqueueReadBuffer(queue_, signatureBuffer, CL_TRUE, 0,
                            numHashes * sizeof(size_t), signature.data(), 0,
                            nullptr, nullptr);

        clReleaseMemObject(hashesBuffer);
        clReleaseMemObject(signatureBuffer);
        clReleaseMemObject(aValuesBuffer);
        clReleaseMemObject(bValuesBuffer);
    }
#endif
};

}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_MHASH_HPP
