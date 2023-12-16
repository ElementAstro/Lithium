#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <vector>

// 基类 Message
class Message
{
public:
    virtual ~Message() = default;

    // 消息类型枚举
    enum class Type
    {
        kText,
        kNumber,
        kBoolean,
        kStructuredData,
        kCustom1,
        kCustom2,
        kMaxType
    };

    Type type() const { return type_; }

protected:
    explicit Message(Type t) : type_(t) {}

private:
    Type type_;

    std::string_view timestamp;
    std::string_view sender;
    std::string_view target;
};

// TextMessage 类
class TextMessage : public Message
{
public:
    explicit TextMessage(std::string text) : Message(Type::kText), text_(std::move(text)) {}
    const std::string &text() const { return text_; }

private:
    std::string text_;
};

// NumberMessage 类
class NumberMessage : public Message
{
public:
    explicit NumberMessage(double number) : Message(Type::kNumber), number_(number) {}
    double number() const { return number_; }

private:
    double number_;
};

// StructuredDataMessage 类
class StructuredDataMessage : public Message
{
public:
    explicit StructuredDataMessage(std::vector<int> data) : Message(Type::kStructuredData), data_(std::move(data)) {}
    const std::vector<int> &data() const { return data_; }

private:
    std::vector<int> data_;
};

// CustomMessage1 类
class CustomMessage1 : public Message
{
public:
    explicit CustomMessage1(std::string data) : Message(Type::kCustom1), data_(std::move(data)) {}
    const std::string &data() const { return data_; }

private:
    std::string data_;
};

// CustomMessage2 类
class CustomMessage2 : public Message
{
public:
    explicit CustomMessage2(int value) : Message(Type::kCustom2), value_(value) {}
    int value() const { return value_; }

private:
    int value_;
};

// Message 类的工厂函数
template <typename T, typename... Args>
std::unique_ptr<T> MakeMessage(Args &&...args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// VariantMessage 类，使用 std::variant 存储各种消息类型
class VariantMessage
{
public:
    VariantMessage() : msg_(TextMessage("")) {}
    template <typename T>
    void Set(T message)
    {
        msg_ = std::move(message);
    }

    template <typename T>
    const T &Get() const
    {
        return std::get<T>(msg_);
    }

    Message::Type type() const
    {
        try
        {
            return static_cast<Message::Type>(msg_.index());
        }
        catch (const std::bad_variant_access &)
        {
            return Message::Type::kMaxType;
        }
    }

private:
    std::variant<
        TextMessage,
        NumberMessage,
        StructuredDataMessage,
        CustomMessage1,
        CustomMessage2>
        msg_;
};

int main()
{
    // 使用工厂函数创建不同类型的消息
    auto text_msg = MakeMessage<TextMessage>("Hello, world!");
    auto number_msg = MakeMessage<NumberMessage>(3.14);
    auto structured_data_msg = MakeMessage<StructuredDataMessage>(std::vector<int>{1, 2, 3});
    auto custom_msg1 = MakeMessage<CustomMessage1>("Custom message 1");
    auto custom_msg2 = MakeMessage<CustomMessage2>(42);

    // 将消息存储在 VariantMessage 中
    VariantMessage variant_msg;
    variant_msg.Set(std::move(*text_msg));

    // 通过 type() 函数获取消息类型，并根据类型进行相应操作
    switch (variant_msg.type())
    {
    case Message::Type::kText:
        std::cout << "Text Message: " << variant_msg.Get<TextMessage>().text() << '\n';
        break;

    case Message::Type::kNumber:
        std::cout << "Number Message: " << variant_msg.Get<NumberMessage>().number() << '\n';
        break;

    case Message::Type::kStructuredData:
        std::cout << "Structured Data Message: ";
        for (const auto &num : variant_msg.Get<StructuredDataMessage>().data())
        {
            std::cout << num << ' ';
        }
        std::cout << '\n';
        break;

    case Message::Type::kCustom1:
        std::cout << "Custom Message 1: " << variant_msg.Get<CustomMessage1>().data() << '\n';
        break;

    case Message::Type::kCustom2:
        std::cout << "Custom Message 2: " << variant_msg.Get<CustomMessage2>().value() << '\n';
        break;

    default:
        std::cout << "Unknown Message Type\n";
        break;
    }

    // 添加更多的消息类型
    variant_msg.Set(std::move(*number_msg));
    variant_msg.Set(std::move(*structured_data_msg));

    // 通过 type() 函数获取消息类型，并根据类型进行相应操作
    switch (variant_msg.type())
    {
    case Message::Type::kText:
        std::cout << "Text Message: " << variant_msg.Get<TextMessage>().text() << '\n';
        break;

    case Message::Type::kNumber:
        std::cout << "Number Message: " << variant_msg.Get<NumberMessage>().number() << '\n';
        break;

    case Message::Type::kStructuredData:
        std::cout << "Structured Data Message: ";
        for (const auto &num : variant_msg.Get<StructuredDataMessage>().data())
        {
            std::cout << num << ' ';
        }
        std::cout << '\n';
        break;

    case Message::Type::kCustom1:
        std::cout << "Custom Message 1: " << variant_msg.Get<CustomMessage1>().data() << '\n';
        break;

    case Message::Type::kCustom2:
        std::cout << "Custom Message 2: " << variant_msg.Get<CustomMessage2>().value() << '\n';
        break;

    default:
        std::cout << "Unknown Message Type\n";
        break;
    }

    return 0;
}
