/*
 * device_exception.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Exception Defination

*************************************************/

#pragma once

#include <stdexcept>
#include <string>

class InvalidDeviceType : public std::exception {
public:
    InvalidDeviceType(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class InvalidParameters : public std::exception {
public:
    InvalidParameters(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class InvalidProperty : public std::exception {
public:
    InvalidProperty(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class InvalidReturn : public std::exception {
public:
    InvalidReturn(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class DispatchError : public std::exception {
public:
    DispatchError(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};