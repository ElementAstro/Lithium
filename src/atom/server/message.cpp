#include "message.hpp"

#include "atom/utils/time.hpp"

Message::Message(Type t, const std::string &target, const std::string &origin)
    : type_(t), target_(target), origin_(origin),timestamp_(GetChinaTimestampString()) {}

// TextMessage class
class TextMessage : public Message
{
public:
    explicit TextMessage(std::string text) : Message(Type::kText), text_(std::move(text)) {}
    const std::string &text() const { return text_; }
    void print() const override { std::cout << "Text Message: " << text_ << '\n'; }

private:
    std::string text_;
};

// NumberMessage class
class NumberMessage : public Message
{
public:
    explicit NumberMessage(double number) : Message(Type::kNumber), number_(number) {}
    double number() const { return number_; }
    void print() const override { std::cout << "Number Message: " << number_ << '\n'; }

private:
    double number_;
};

// StructuredDataMessage class
class StructuredDataMessage : public Message
{
public:
    explicit StructuredDataMessage(std::vector<int> data) : Message(Type::kStructuredData), data_(std::move(data)) {}
    const std::vector<int> &data() const { return data_; }
    void print() const override
    {
        std::cout << "Structured Data Message: ";
        for (const auto &num : data_)
        {
            std::cout << num << ' ';
        }
        std::cout << '\n';
    }

private:
    std::vector<int> data_;
};

// CustomMessage1 class
class CustomMessage1 : public Message
{
public:
    explicit CustomMessage1(std::string data) : Message(Type::kCustom1), data_(std::move(data)) {}
    const std::string &data() const { return data_; }
    void print() const override { std::cout << "Custom Message 1: " << data_ << '\n'; }

private:
    std::string data_;
};

// CustomMessage2 class
class CustomMessage2 : public Message
{
public:
    explicit CustomMessage2(int value) : Message(Type::kCustom2), value_(value) {}
    int value() const { return value_; }
    void print() const override { std::cout << "Custom Message 2: " << value_ << '\n'; }

private:
    int value_;
};

// Message factory function
template <typename T, typename... Args>
std::unique_ptr<T> MakeMessage(Args &&...args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// VariantMessage class, using std::unique_ptr<Message> to store different message types
class VariantMessage
{
public:
    VariantMessage() : msg_(std::make_unique<TextMessage>("")) {}

    template <typename T>
    void Set(T message)
    {
        msg_ = std::make_unique<T>(std::move(message));
    }

    template <typename T>
    const T &Get() const
    {
        return static_cast<const T &>(*msg_);
    }

    std::optional<Message::Type> type() const
    {
        if (msg_)
        {
            return msg_->type();
        }
        return std::nullopt;
    }

    void reset()
    {
        msg_ = std::make_unique<TextMessage>("");
    }

private:
    std::unique_ptr<Message> msg_;
};

int main()
{
    // Create different types of messages using the factory function
    auto text_msg = MakeMessage<TextMessage>("Hello, world!");
    auto number_msg = MakeMessage<NumberMessage>(3.14);
    auto structured_data_msg = MakeMessage<StructuredDataMessage>(std::vector<int>{1, 2, 3});
    auto custom_msg1 = MakeMessage<CustomMessage1>("Custom message 1");
    auto custom_msg2 = MakeMessage<CustomMessage2>(42);

    // Store messages in VariantMessage
    VariantMessage variant_msg;
    variant_msg.Set(std::move(*text_msg));

    // Print the message based on its type
    if (auto type = variant_msg.type())
    {
        switch (*type)
        {
        case Message::Type::kText:
            variant_msg.Get<TextMessage>().print();
            break;
        case Message::Type::kNumber:
            variant_msg.Get<NumberMessage>().print();
            break;
        case Message::Type::kStructuredData:
            variant_msg.Get<StructuredDataMessage>().print();
            break;
        case Message::Type::kCustom1:
            variant_msg.Get<CustomMessage1>().print();
            break;
        case Message::Type::kCustom2:
            variant_msg.Get<CustomMessage2>().print();
            break;
        default:
            std::cout << "Unknown Message Type\n";
            break;
        }
    }
    else
    {
        std::cout << "No Message Set\n";
    }

    // Add more messages to VariantMessage
    variant_msg.Set(std::move(*number_msg));
    variant_msg.Set(std::move(*structured_data_msg));

    variant_msg.reset();
    variant_msg.Set(std::move(*structured_data_msg));

    // Print the new message based on its type
    if (auto type = variant_msg.type())
    {
        switch (*type)
        {
        case Message::Type::kText:
            variant_msg.Get<TextMessage>().print();
            break;
        case Message::Type::kNumber:
            variant_msg.Get<NumberMessage>().print();
            break;
        case Message::Type::kStructuredData:
            variant_msg.Get<StructuredDataMessage>().print();
            break;
        case Message::Type::kCustom1:
            variant_msg.Get<CustomMessage1>().print();
            break;
        case Message::Type::kCustom2:
            variant_msg.Get<CustomMessage2>().print();
            break;
        default:
            std::cout << "Unknown Message Type\n";
            break;
        }
    }
    else
    {
        std::cout << "No Message Set\n";
    }

    return 0;
}
