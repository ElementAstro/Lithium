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
#include <pybind11/stl_bind.h>

#include "algorithm.hpp"
#include "base.hpp"
#include "convolve.hpp"
#include "fraction.hpp"
#include "hash.hpp"
#include "huffman.hpp"
#include "math.hpp"
#include "md5.hpp"

namespace py = pybind11;

using namespace Atom::Algorithm;

PYBIND11_MODULE(atom_algorithm, m) {
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

    py::class_<Fraction>(m, "Fraction")
        .def(py::init<int, int>(), py::arg("numerator") = 0,
             py::arg("denominator") = 1)
        .def("__add__", &Fraction::operator+, py::arg("other"))
        //.def("__sub__", &Fraction::operator-, py::arg("other"))
        .def("__mul__", &Fraction::operator*, py::arg("other"))
        .def("__truediv__", &Fraction::operator/, py::arg("other"))
        .def(py::self += py::self)
        .def(py::self -= py::self)
        .def(py::self *= py::self)
        .def(py::self /= py::self)
        .def("__eq__", &Fraction::operator==)
        //.def("__lt__", &Fraction::operator<)
        //.def("__neg__", &Fraction::operator-)
        .def("__str__", &Fraction::to_string)
        .def("__float__", &Fraction::to_double)
        .def("__int__", &Fraction::operator int)
        .def("__float__", &Fraction::operator float)
        .def("__double__", &Fraction::operator double)
        .def("__repr__", &Fraction::to_string)
        .def("__abs__",
             [](const Fraction& f) {
                 return Fraction(std::abs(f.numerator),
                                 std::abs(f.denominator));
             })
        .def(
            "__iadd__",
            [](Fraction& self, const Fraction& other) {
                self += other;
                return self;
            },
            py::arg("other"))
        .def(
            "__isub__",
            [](Fraction& self, const Fraction& other) {
                self -= other;
                return self;
            },
            py::arg("other"))
        .def(
            "__imul__",
            [](Fraction& self, const Fraction& other) {
                self *= other;
                return self;
            },
            py::arg("other"))
        .def(
            "__idiv__",
            [](Fraction& self, const Fraction& other) {
                self /= other;
                return self;
            },
            py::arg("other"))
        .def("__pos__",
             [](const Fraction& f) {
                 return Fraction(f.numerator, f.denominator);
             })
        .def("__abs__",
             [](const Fraction& f) {
                 return Fraction(std::abs(f.numerator),
                                 std::abs(f.denominator));
             })
        .def("__pos__",
             [](const Fraction& f) {
                 return Fraction(f.numerator, f.denominator);
             })
        .def("__neg__",
             [](const Fraction& f) {
                 return Fraction(-f.numerator, f.denominator);
             })
        .def("__hash__",
             [](const Fraction& f) {
                 return std::hash<double>()(f.to_double());
             })
        .def(
            "__pow__",
            [](const Fraction& f, int power) {
                return Fraction(std::pow(f.to_double(), power));
            },
            py::arg("power"))
        //.def(py::self < py::self)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__float__", [](const Fraction& f) { return f.to_double(); })
        .def("__int__",
             [](const Fraction& f) { return static_cast<int>(f.to_double()); });

    //m.def("compute_hash", &fnv1a_hash<std::string_view>,
    //      "Compute FNV-1a hash for a string view");

    //m.def("compute_hash", &jenkins_one_at_a_time_hash<std::string_view>,
    //      "Compute Jenkins One-at-a-Time hash for a string view");

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

    m.def("mul_div_64", &mulDiv64, "Perform 64-bit multiplication and division");

    py::class_<MD5>(m, "MD5")
        .def(py::init<>())
        .def_static("encrypt", &MD5::encrypt);
}
