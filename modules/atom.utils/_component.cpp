/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-Utils

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"

#include "atom/utils/aes.hpp"
#include "atom/utils/convert.hpp"

using namespace atom::utils;

UtilsComponent::UtilsComponent(const std::string &name) : Component(name) {
    DLOG_F(INFO, "UtilsComponent::UtilsComponent");

    def("encrypt_aes", &encryptAES, "utils", "encrypt a string with AES");
    def("decrypt_aes", &decryptAES, "utils", "decrypt a string with AES");
    def("compress", &compress, "utils", "compress a string");
    def("decompress", &decompress, "utils", "decompress a string");
}

UtilsComponent::~UtilsComponent() {
    DLOG_F(INFO, "UtilsComponent::~UtilsComponent");
}

bool UtilsComponent::initialize() { return true; }

bool UtilsComponent::destroy() { return true; }
