/*
 * fitskeyword.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: FITS Keyword

**************************************************/

#include "fitskeyword.hpp"

#include <cmath>
#include <sstream>

FITSRecord::FITSRecord() : val_int64(0), m_type(VOID) {}

FITSRecord::FITSRecord(const char *key, const char *value, const char *comment)
    : m_key(key), m_type(STRING) {
    if (value)
        val_str = std::string(value);

    if (comment)
        m_comment = std::string(comment);
}

FITSRecord::FITSRecord(const char *key, int64_t value, const char *comment)
    : val_int64(value),
      val_str(std::to_string(value)),
      m_key(key),
      m_type(LONGLONG) {
    if (comment)
        m_comment = std::string(comment);
}

FITSRecord::FITSRecord(const char *key, double value, int decimal,
                       const char *comment)
    : val_double(value), m_key(key), m_type(DOUBLE), m_decimal(decimal) {
    std::stringstream ss;
    ss.precision(decimal);
    ss << value;
    val_str = ss.str();

    if (comment)
        m_comment = std::string(comment);
}

FITSRecord::FITSRecord(const char *comment)
    : m_key("COMMENT"), m_type(COMMENT) {
    if (comment)
        m_comment = std::string(comment);
}

FITSRecord::Type FITSRecord::type() const { return m_type; }

const std::string &FITSRecord::key() const { return m_key; }

const std::string &FITSRecord::valueString() const { return val_str; }

int64_t FITSRecord::valueInt() const {
    if (m_type == LONGLONG)
        return val_int64;
    else
        return 0;
}

double FITSRecord::valueDouble() const {
    if (m_type == DOUBLE)
        return val_double;
    else
        return NAN;
}

const std::string &FITSRecord::comment() const { return m_comment; }

int FITSRecord::decimal() const { return m_decimal; }
