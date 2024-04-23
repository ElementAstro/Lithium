#include "carbon/carbon.hpp"

#include "algorithm.hpp"
#include "base.hpp"
#include "convolve.hpp"
#include "fraction.hpp"
#include "hash.hpp"
#include "huffman.hpp"
#include "math.hpp"
#include "md5.hpp"
#include "mhash.hpp"

using namespace Atom::Algorithm;
using namespace Atom::Utils;

namespace Atom::_Script::Algorithm {
/**
 * Adds the String Methods to the given Carbon m.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {
    m->add(user_type<KMP>(), "KMP");
    m->add(Carbon::fun(&KMP::Search), "search");
    m->add(Carbon::fun(&KMP::SetPattern), "set_pattern");

    m->add(user_type<MinHash>(), "MinHash");
    m->add(Carbon::fun(&MinHash::compute_signature), "compute_signature");
    m->add(Carbon::fun(&MinHash::estimate_similarity), "estimate_similarity");

    m->add(user_type<BoyerMoore>(), "BoyerMoore");
    m->add(Carbon::fun(&BoyerMoore::Search), "search");
    m->add(Carbon::fun(&BoyerMoore::SetPattern), "set_pattern");

    m->add(user_type<BloomFilter<16>>(), "BloomFilter");
    m->add(Carbon::fun(&BloomFilter<16>::insert), "insert");
    m->add(Carbon::fun(&BloomFilter<16>::contains), "contains");

    m->add(Carbon::fun(&base16Encode), "base16encode");
    m->add(Carbon::fun(&base16Decode), "base16decode");
    m->add(Carbon::fun(&base32Encode), "base32encode");
    m->add(Carbon::fun(&base32Decode), "base32decode");
    m->add(Carbon::fun(&base64Encode), "base64encode");
    m->add(Carbon::fun(&base64Decode), "base64decode");
    m->add(Carbon::fun(&base85Encode), "base85encode");
    m->add(Carbon::fun(&base85Decode), "base85decode");
    m->add(Carbon::fun(&base91Encode), "base91encode");
    m->add(Carbon::fun(&base91Decode), "base91decode");
    m->add(Carbon::fun(&base128Encode), "base128encode");
    m->add(Carbon::fun(&base128Decode), "base128decode");

    m->add(Carbon::fun(&xorEncrypt), "xor_encrypt");
    m->add(Carbon::fun(&xorDecrypt), "xor_decrypt");

    m->add(Carbon::fun(&convolve), "convolve");
    m->add(Carbon::fun(&deconvolve), "deconvolve");
    m->add(Carbon::fun(&convolve2D), "convolve2d");
    m->add(Carbon::fun(&deconvolve2D), "deconvolve2d");

    m->add(user_type<Fraction>(), "Fraction");
    //m->add(Carbon::constructor<Fraction(int, int)>(), "Fraction");
    m->add(Carbon::fun(&Fraction::operator+=), "+=");
    m->add(Carbon::fun(&Fraction::operator-=), "-=");
    m->add(Carbon::fun(&Fraction::operator*=), "*=");
    m->add(Carbon::fun(&Fraction::operator/=), "/=");
    m->add(Carbon::fun(&Fraction::operator+), "+");
    m->add(Carbon::fun<Fraction (Fraction::*)() const>(&Fraction::operator-),
           "-");
    m->add(Carbon::fun<Fraction (Fraction::*)() const>(&Fraction::operator-),
           "-");
    m->add(Carbon::fun<Fraction (Fraction::*)(const Fraction &) const>(
               &Fraction::operator-),
           "-");
    m->add(Carbon::fun(&Fraction::operator*), "*");
    m->add(Carbon::fun(&Fraction::operator/), "/");
    m->add(Carbon::fun(&Fraction::operator==), "==");
    m->add(
        Carbon::fun<double (Fraction::*)() const>(&Fraction::operator double),
        "to_double");
    m->add(Carbon::fun<float (Fraction::*)() const>(&Fraction::operator float),
           "to_float");
    m->add(Carbon::fun<int (Fraction::*)() const>(&Fraction::operator int),
           "to_int");
    m->add(Carbon::fun(&Fraction::to_string), "to_string");
    m->add(Carbon::fun(&Fraction::to_double), "to_double");

    m->add(Carbon::fun(static_cast<std::uint32_t (*)(const void *, size_t)>(
               &quickHash)),
           "hash");
    m->add(Carbon::fun(
               static_cast<std::uint32_t (*)(std::string_view)>(&quickHash)),
           "hash");

    m->add(user_type<HuffmanNode>(), "HuffmanNode");
    //m->add(Carbon::constructor<HuffmanNode(char, int)>(), "HuffmanNode");
    m->add(Carbon::fun(&HuffmanNode::data), "data");
    m->add(Carbon::fun(&HuffmanNode::frequency), "frequency");
    m->add(Carbon::fun(&HuffmanNode::left), "left");
    m->add(Carbon::fun(&HuffmanNode::right), "right");

    m->add(Carbon::fun(&createHuffmanTree), "create_huffman_tree");
    m->add(Carbon::fun(&generateHuffmanCodes), "generate_huffman_codes");
    m->add(Carbon::fun(&compressText), "compress_text");
    m->add(Carbon::fun(&decompressText), "decompress_text");

    m->add(Carbon::fun(&mulDiv64), "mul_div_64");

    m->add(Carbon::fun(&MD5::encrypt), "md5_encrypt");

    m->add(
        Carbon::fun<uint32_t (*)(const char *, const uint32_t &)>(&murmur3Hash),
        "murmur3_hash");
    m->add(Carbon::fun<uint64_t (*)(const char *, const uint32_t &,
                                    const uint32_t &)>(&murmur3Hash64),
           "murmur3_hash_64");
    m->add(
        Carbon::fun<std::string (*)(const std::string &)>(&dataFromHexstring),
        "data_from_hexstring");
    m->add(
        Carbon::fun<std::string (*)(const char *, size_t)>(&dataFromHexstring),
        "data_from_hexstring");
    m->add(
        Carbon::fun<std::string (*)(const std::string &)>(&hexstringFromData),
        "hexstring_from_data");
    return m;
}
}  // namespace Atom::_Script::Algorithm
