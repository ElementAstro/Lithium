/*
 * component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-Utils

**************************************************/

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/utils/aes.hpp"
#include "atom/utils/convert.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using namespace atom::utils;

ATOM_MODULE(atom_utils, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    component.def("encrypt_aes", &encryptAES, "utils",
                  "encrypt a string with AES");
    component.def("decrypt_aes", &decryptAES, "utils",
                  "decrypt a string with AES");
    component.def("compress", &compress, "utils", "compress a string");
    component.def("decompress", &decompress, "utils", "decompress a string");
    component.def("calculate_sha256", &calculateSha256, "utils",
                  "calculate SHA256 hash of a string");
    component.def("calculate_sha224", &calculateSha224, "utils",
                  "calculate SHA224 hash of a string");
    component.def("calculate_sha384", &calculateSha384, "utils",
                  "calculate SHA384 hash of a string");
    component.def("calculate_sha512", &calculateSha512, "utils",
                  "calculate SHA512 hash of a string");
    LOG_F(INFO, "Loaded module {}", component.getName());
});
