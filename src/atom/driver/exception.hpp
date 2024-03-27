/*
 * exception.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Exception Defination

*************************************************/

#ifndef ATOM_DRIVER_EXCEPTION_HPP
#define ATOM_DRIVER_EXCEPTION_HPP

#include <stdexcept>
#include <string>

class InvalidDeviceType : public std::exception {
public:
    explicit InvalidDeviceType(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class InvalidParameters : public std::exception {
public:
    explicit InvalidParameters(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class InvalidProperty : public std::exception {
public:
    explicit InvalidProperty(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class InvalidReturn : public std::exception {
public:
    explicit InvalidReturn(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

class DispatchError : public std::exception {
public:
    explicit DispatchError(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};

#endif
