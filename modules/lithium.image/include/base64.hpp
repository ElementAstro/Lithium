#ifndef LITHIUM_IMAGE_BASE64_HPP
#define LITHIUM_IMAGE_BASE64_HPP

#include <string>

std::string base64_encode(unsigned char const* bytes_to_encode,
                          unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);
#endif
