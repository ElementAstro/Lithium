/*
 * message.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-18

Description: A message class, which can be used to store different types of
messages

**************************************************/

#include "message.hpp"

#include "atom/utils/time.hpp"
#include "atom/utils/uuid.hpp"

using namespace std;

// Message
Message::Message(Type t, const string &name, const string &target,
                 const string &origin)
    : type_(t), target_(target), origin_(origin), name_(name) {
    timestamp_ = atom::utils::getChinaTimestampString();
    atom::utils::UUID generator;
    uuid_ = generator.toString();
}

Message::Type Message::fromInt(const int &t) {
    switch (t) {
        case 0:
            return Type::kVoid;
        case 1:
            return Type::kText;
        case 2:
            return Type::kNumber;
        case 3:
            return Type::kBoolean;
        case 4:
            return Type::kAny;
        case 5:
            return Type::kParams;
        default:
            return Type::kMaxType;
    }
}

Message::Type Message::type() const { return type_; }

string_view Message::target() const { return target_; }

string_view Message::origin() const { return origin_; }

string Message::timestamp() const {
    // TODO: implement timestamp function
    return "";
}

string Message::name() const { return name_; }

double Message::api_version() const { return api_version_; }

// Void Message
VoidMessage::VoidMessage(const string &name, const string &target,
                         const string &origin, bool has_return)
    : Message(Type::kVoid, name, target, origin), has_return(has_return) {}

bool VoidMessage::hasReturn() const { return has_return; }
// TextMessage
TextMessage::TextMessage(const string &name, const string &text,
                         const string &target, const string &origin)
    : Message(Type::kText, name, target, origin), value_(text) {}

string TextMessage::value() const { return value_; }

// NumberMessage
NumberMessage::NumberMessage(const string &name, double number,
                             const string &target, const string &origin)
    : Message(Type::kNumber, name, target, origin), value_(number) {}

double NumberMessage::value() const { return value_; }

// BooleanMessage
BooleanMessage::BooleanMessage(const string &name, bool value,
                               const string &target, const string &origin)
    : Message(Type::kBoolean, name, target, origin), value_(value) {}

bool BooleanMessage::value() const { return value_; }

// AnyMessage
AnyMessage::AnyMessage(const string &name, const any &data,
                       const string &target, const string &origin)
    : Message(Type::kAny, name, target, origin), data_(data) {
    type_ = data_.type().name();
}

any AnyMessage::value() const { return data_; }

string AnyMessage::type() const { return type_; }

// ParamsMessage
ParamsMessage::ParamsMessage(const string &name, const Args &params,
                             const string &target, const string &origin)
    : Message(Type::kParams, name, target, origin), params_(params) {}

Args ParamsMessage::value() const { return params_; }

shared_ptr<VoidMessage> MessageHelper::MakeVoidMessage(const string &name,
                                                       const string &target,
                                                       const string &origin) {
    return make_shared<VoidMessage>(name, target, origin, true);
}

// MessageHelper
shared_ptr<TextMessage> MessageHelper::MakeTextMessage(const string &name,
                                                       const string &value,
                                                       const string &target,
                                                       const string &origin) {
    return make_shared<TextMessage>(name, value, target, origin);
}

shared_ptr<NumberMessage> MessageHelper::MakeNumberMessage(
    const string &name, double value, const string &target,
    const string &origin) {
    return make_shared<NumberMessage>(name, value, target, origin);
}

shared_ptr<BooleanMessage> MessageHelper::MakeBooleanMessage(
    const string &name, bool value, const string &target,
    const string &origin) {
    return make_shared<BooleanMessage>(name, value, target, origin);
}

shared_ptr<AnyMessage> MessageHelper::MakeAnyMessage(const string &name,
                                                     const any &value,
                                                     const string &target,
                                                     const string &origin) {
    return make_shared<AnyMessage>(name, value, target, origin);
}

shared_ptr<ParamsMessage> MessageHelper::MakeParamsMessage(
    const string &name, const Args &params, const string &target,
    const string &origin) {
    return make_shared<ParamsMessage>(name, params, target, origin);
}
