#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

// Base class Message
class Message
{
public:
    virtual ~Message() = default;

    // Message type enumeration
    enum class Type
    {
        kText,
        kNumber,
        kBoolean,
        kSwitch,
        kAny,
        kMaxType
    };

    Type type() const { return type_; }

    // Common operations that can be performed on all message types
    virtual void serialize() const = 0;

protected:
    explicit Message(Type t, const std::string &target, const std::string &origin);

private:
    Type type_;
    std::string_view target_;
    std::string_view origin_;

    std::string_view timestamp_;
    double api_version_{1.0};
};

// TextMessage class
class TextMessage : public Message
{
public:
    explicit TextMessage(std::string text,const std::string &target, const std::string &origin);
    void serialize() const override;

private:
    std::string text_;
};

// NumberMessage class
class NumberMessage : public Message
{
public:
    explicit NumberMessage(double number) : Message(Type::kNumber), number_(number) {}
    void serialize() const override;

private:
    double number_;
};

// StructuredDataMessage class
class StructuredDataMessage : public Message
{
public:
    explicit StructuredDataMessage(std::vector<int> data) : Message(Type::kStructuredData), data_(std::move(data)) {}
    void serialize() const override;

private:
    std::vector<int> data_;
};

class SwitchMessage : public Message
{
public:
    explicit SwitchMessage(std::string name, std::string value) : Message(Type::kSwitch), name_(std::move(name)), value_(std::move(value)) {}
    void serialize() const override;

private:
    std::string name_;
    std::string value_;
};

class AnyMessage : public Message
{
public:
    explicit AnyMessage(std::string data) : Message(Type::kAny), data_(std::move(data)) {}
    void serialize() const override;

private:
    std::string data_;
};

// Message factory function
template <typename T, typename... Args>
std::unique_ptr<T> MakeUniqueMessage(Args &&...args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Message factory function
template <typename T, typename... Args>
std::unique_ptr<T> MakeSharedMessage(Args &&...args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
