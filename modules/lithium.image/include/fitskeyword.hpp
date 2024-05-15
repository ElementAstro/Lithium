/*
 * fitskeyword.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: FITS Keyword

**************************************************/

#ifndef LITHIUM_IMAGE_FITSKEYWORD_HPP
#define LITHIUM_IMAGE_FITSKEYWORD_HPP

#include <fitsio.h>
#include <cstdint>
#include <string>

class FITSRecord {
public:
    enum Type {
        VOID,
        COMMENT,
        STRING = TSTRING,
        LONGLONG = TLONGLONG,
        // ULONGLONG = TULONGLONG,
        DOUBLE = TDOUBLE
    };
    FITSRecord();
    FITSRecord(const char *key, const char *value,
               const char *comment = nullptr);
    FITSRecord(const char *key, int64_t value, const char *comment = nullptr);
    // FITSRecord(const char *key, uint64_t value, const char *comment =
    // nullptr);
    FITSRecord(const char *key, double value, int decimal = 6,
               const char *comment = nullptr);
    explicit FITSRecord(const char *comment);
    Type type() const;
    const std::string &key() const;
    const std::string &valueString() const;
    int64_t valueInt() const;
    // uint64_t valueUInt() const;
    double valueDouble() const;
    const std::string &comment() const;
    int decimal() const;

private:
    union {
        int64_t val_int64;
        uint64_t val_uint64;
        double val_double;
    };
    std::string val_str;
    std::string m_key;
    Type m_type = VOID;
    std::string m_comment;
    int m_decimal = 6;
};

#endif