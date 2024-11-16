#ifndef LITHIUM_IMAGE_BASE64_HPP
#define LITHIUM_IMAGE_BASE64_HPP

#include <string>

auto base64Encode(unsigned char const* bytes_to_encode,
                  unsigned int input_length) -> std::string;

auto base64Decode(std::string const& encoded_string) -> std::string;

#endif
