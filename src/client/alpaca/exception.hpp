#pragma once

#include <stdexcept>
#include <string>

class ActionNotImplementedException : public std::exception
{
public:
    ActionNotImplementedException(const std::string &message) : number(0x40C), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class AlpacaRequestException : public std::exception
{
public:
    AlpacaRequestException(int statusCode, const std::string &responseText, const std::string &url) : errorMessage(responseText + " " + url) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    std::string errorMessage;
};

class DriverException : public std::exception
{
public:
    DriverException(int errorCode, const std::string &message) : number(errorCode), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class InvalidOperationException : public std::exception
{
public:
    InvalidOperationException(const std::string &message) : number(0x40B), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class InvalidValueException : public std::exception
{
public:
    InvalidValueException(const std::string &message) : number(0x401), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class NotConnectedException : public std::exception
{
public:
    NotConnectedException(const std::string &message) : number(0x407), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class NotImplementedException : public std::exception
{
public:
    NotImplementedException(const std::string &message) : number(0x400), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class ParkedException : public std::exception
{
public:
    ParkedException(const std::string &message) : number(0x408), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class SlavedException : public std::exception
{
public:
    SlavedException(const std::string &message) : number(0x409), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};

class ValueNotSetException : public std::exception
{
public:
    ValueNotSetException(const std::string &message) : number(0x402), errorMessage(message) {}
    const char *what() const noexcept override { return errorMessage.c_str(); }

private:
    int number;
    std::string errorMessage;
};
