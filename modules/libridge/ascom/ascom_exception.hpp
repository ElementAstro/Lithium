#pragma once

#include <exception>
#include <string>

class ActionNotImplementedException : public std::exception
{
public:
    ActionNotImplementedException(const std::string &message) : message_(message) {}

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
    AlpacaRequestException(int number, const std::string &message) : number_(number), message_(message) {}

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
    DriverException(int number, const std::string &message) : number_(number), message_(message) {}

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
    InvalidOperationException(const std::string &message) : message_(message) {}

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
    InvalidValueException(const std::string &message) : message_(message) {}

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
    NotConnectedException(const std::string &message) : message_(message) {}

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
    NotImplementedException(const std::string &message) : message_(message) {}

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
    ParkedException(const std::string &message) : message_(message) {}

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
    SlavedException(const std::string &message) : message_(message) {}

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
    ValueNotSetException(const std::string &message) : message_(message) {}

    const char *what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};
