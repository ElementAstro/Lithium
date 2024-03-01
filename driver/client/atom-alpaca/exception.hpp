/*
 * exception.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: the Same Expectations like Alpaca

**************************************************/

#ifndef ATOM_ALPACA_EXCEPTION_HPP
#define ATOM_ALPACA_EXCEPTION_HPP

#include <exception>
#include <string>

class ActionNotImplementedException : public std::exception
{
public:
    explicit ActionNotImplementedException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class AlpacaRequestException : public std::exception
{
public:
    explicit AlpacaRequestException(int number, const std::string &message) : number_(number), message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

    int number() const
    {
        return number_;
    }

private:
    int number_;
    std::string message_;
};

class DriverException : public std::exception
{
public:
    explicit DriverException(int number, const std::string &message) : number_(number), message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

    int number() const
    {
        return number_;
    }

private:
    int number_;
    std::string message_;
};

class InvalidOperationException : public std::exception
{
public:
    explicit InvalidOperationException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class InvalidValueException : public std::exception
{
public:
    explicit InvalidValueException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class NotConnectedException : public std::exception
{
public:
    explicit NotConnectedException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class NotImplementedException : public std::exception
{
public:
    explicit NotImplementedException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class ParkedException : public std::exception
{
public:
    explicit ParkedException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class SlavedException : public std::exception
{
public:
    explicit SlavedException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

class ValueNotSetException : public std::exception
{
public:
    explicit ValueNotSetException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

#endif
