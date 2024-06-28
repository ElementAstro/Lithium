/*
 * _pybind.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Python Binding of Atom-Algorithm

**************************************************/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "algorithm.hpp"
#include "base.hpp"
#include "convolve.hpp"
#include "fraction.hpp"
#include "hash.hpp"
#include "huffman.hpp"
#include "math.hpp"
#include "md5.hpp"
#include "mhash.hpp"

namespace py = pybind11;

using namespace atom::algorithm;

PYBIND11_EMBEDDED_MODULE(atom_algorithm, m) {
    m.doc() = "Atom Algorithm Python Binding";

    m.def("base16encode", &base16Encode, "Base16 Encode");
    m.def("base16decode", &base16Decode, "Base16 Decode");
    m.def("base32encode", &base32Encode, "Base32 Encode");
    m.def("base32decode", &base32Decode, "Base32 Decode");
    m.def("base64encode", &base64Encode, "Base64 Encode");
    m.def("base64decode", &base64Decode, "Base64 Decode");
    m.def("base85encode", &base85Encode, "Base85 Encode");
    m.def("base85decode", &base85Decode, "Base85 Decode");
    m.def("base91encode", &base91Encode, "Base91 Encode");
    m.def("base91decode", &base91Decode, "Base91 Decode");
    m.def("base128encode", &base128Encode, "Base128 Encode");
    m.def("base128decode", &base128Decode, "Base128 Decode");

    py::class_<KMP>(m, "KMP")
        .def(py::init<std::string_view>())
        .def("search", &KMP::Search)
        .def("set_pattern", &KMP::SetPattern);

    py::class_<MinHash>(m, "MinHash")
        .def(py::init<int>())
        .def("compute_signature", &MinHash::compute_signature)
        .def("estimate_similarity", &MinHash::estimate_similarity);

    m.def("convolve", &convolve, "Perform one-dimensional convolution");
    m.def("deconvolve", &deconvolve, "Perform one-dimensional deconvolution");
    m.def("convolve2D", &convolve2D, "Perform two-dimensional convolution",
          py::arg("input"), py::arg("kernel"), py::arg("numThreads") = 1);
    m.def("deconvolve2D", &deconvolve2D,
          "Perform two-dimensional deconvolution");
    m.def("DFT2D", &DFT2D, "Perform two-dimensional discrete Fourier transform",
          py::arg("signal"), py::arg("numThreads") = 1);
    m.def("IDFT2D", &IDFT2D,
          "Perform two-dimensional inverse discrete Fourier transform",
          py::arg("spectrum"), py::arg("numThreads") = 1);
    m.def("generate_gaussian_kernel", &generateGaussianKernel,
          "Generate a Gaussian kernel for convolution");
    m.def("apply_gaussian_filter", &applyGaussianFilter,
          "Apply a Gaussian filter to an image");

    py::class_<Fraction>(m, "Fraction")
        .def(py::init<int, int>())
        .def_readwrite("numerator", &Fraction::numerator)
        .def_readwrite("denominator", &Fraction::denominator)
        .def("__add__", &Fraction::operator+, py::is_operator())
        .def("__sub__", &Fraction::operator-, py::is_operator())
        .def("__mul__", &Fraction::operator*, py::is_operator())
        .def("__truediv__", &Fraction::operator/, py::is_operator())
        .def("__eq__", &Fraction::operator==, py::is_operator())
        .def("__neg__", &Fraction::operator-)
        .def("__str__", &Fraction::to_string)
        .def("__float__", &Fraction::to_double)
        .def("__int__", &Fraction::operator int)
        .def("__repr__", [](const Fraction &f) { return f.to_string(); });

    // Bindings for createHuffmanTree function
    m.def("create_huffman_tree", &createHuffmanTree,
          "Create a Huffman tree from character frequencies");

    // Bindings for generateHuffmanCodes function
    m.def("generate_huffman_codes", &generateHuffmanCodes,
          "Generate Huffman codes for characters in the Huffman tree");

    // Bindings for compressText function
    m.def("compress_text", &compressText,
          "Compress text using Huffman encoding");

    // Bindings for decompressText function
    m.def("decompress_text", &decompressText,
          "Decompress text using Huffman decoding");

    m.def("mul_div_64", &mulDiv64,
          "Perform 64-bit multiplication and division");
    m.def("safe_add", &safeAdd, "Safe addition");
    m.def("safe_sub", &safeSub, "Safe subtraction");
    m.def("safe_mul", &safeMul, "Safe multiplication");
    m.def("safe_div", &safeDiv, "Safe division");
    m.def("normalize", &normalize, "Normalize a 64-bit number");
    m.def("rotl64", &rotl64, "Rotate left");
    m.def("rotr64", &rotr64, "Rotate right");
    m.def("clz64", &clz64, "Count leading zeros");

    py::class_<MD5>(m, "MD5")
        .def(py::init<>())
        .def_static("encrypt", &MD5::encrypt);

    m.def("murmur3_hash", &murmur3Hash, "Murmur3 Hash");
    m.def("murmur3_hash64", &murmur3Hash64, "Murmur3 Hash64");
    m.def("hexstring_from_data",
          py::overload_cast<const std::string &>(&hexstringFromData),
          "Hexstring from Data");
    m.def("data_from_hexstring", &dataFromHexstring, "Data from Hexstring");
}
