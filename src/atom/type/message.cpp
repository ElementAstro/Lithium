/*
 * message.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-18

Description: A message class, which can be used to store different types of messages

**************************************************/

#include "message.hpp"

#include "atom/utils/time.hpp"
#include "atom/type/iparams.hpp"

#include "core/property/uuid.hpp"

using namespace std;

// Message
Message::Message(Type t, const string &name, const string &target, const string &origin)
    : type_(t), name_(name), target_(target), origin_(origin)
{
    timestamp_ = GetChinaTimestampString();
    uuid_ = LITHIUM::UUID::UUIDGenerator::generateUUIDWithFormat();
}

Message::Type Message::type() const
{
    return type_;
}

string_view Message::target() const
{
    return target_;
}

string_view Message::origin() const
{
    return origin_;
}

string Message::timestamp() const
{
    // TODO: implement timestamp function
    return "";
}

string Message::name() const
{
    return name_;
}

double Message::api_version() const
{
    return api_version_;
}

// Void Message
VoidMessage::VoidMessage(const string &name, const string &target, const string &origin)
    : Message(Type::kVoid, name, target, origin)
{
}

// TextMessage
TextMessage::TextMessage(const string &name, const string &text, const string &target, const string &origin)
    : Message(Type::kText, name, target, origin), value_(text)
{
}

string TextMessage::value() const
{
    return value_;
}

// NumberMessage
NumberMessage::NumberMessage(const string &name, double number, const string &target, const string &origin)
    : Message(Type::kNumber, name, target, origin), value_(number)
{
}

double NumberMessage::value() const
{
    return value_;
}

// BooleanMessage
BooleanMessage::BooleanMessage(const string &name, bool value, const string &target, const string &origin)
    : Message(Type::kBoolean, name, target, origin), value_(value)
{
}

bool BooleanMessage::value() const
{
    return value_;
}

// AnyMessage
AnyMessage::AnyMessage(const string &name, const any &data, const string &target, const string &origin)
    : Message(Type::kAny, name, target, origin), data_(data)
{
    type_ = data_.type().name();
}

any AnyMessage::value() const
{
    return data_;
}

string AnyMessage::type() const
{
    return type_;
}

// ParamsMessage
ParamsMessage::ParamsMessage(const string &name, shared_ptr<IParams> params, const string &target, const string &origin)
    : Message(Type::kParams, name, target, origin), params_(params)
{
}

shared_ptr<IParams> ParamsMessage::value() const
{
    return params_;
}

// JsonMessage
JsonMessage::JsonMessage(const string &name, const json &json, const string &target, const string &origin)
    : Message(Type::kJson, name, target, origin), value_(json)
{
}

json JsonMessage::value() const
{
    return value_;
}

// MessageHelper
shared_ptr<TextMessage> MessageHelper::MakeTextMessage(const string &name, const string &value, const string &target, const string &origin)
{
    return make_shared<TextMessage>(name, value, target, origin);
}

shared_ptr<NumberMessage> MessageHelper::MakeNumberMessage(const string &name, double value, const string &target, const string &origin)
{
    return make_shared<NumberMessage>(name, value, target, origin);
}

shared_ptr<BooleanMessage> MessageHelper::MakeBooleanMessage(const string &name, bool value, const string &target, const string &origin)
{
    return make_shared<BooleanMessage>(name, value, target, origin);
}

shared_ptr<AnyMessage> MessageHelper::MakeAnyMessage(const string &name, const any &data, const string &target, const string &origin)
{
    return make_shared<AnyMessage>(name, data, target, origin);
}

shared_ptr<ParamsMessage> MessageHelper::MakeParamsMessage(const string &name, shared_ptr<IParams> params, const string &target, const string &origin)
{
    return make_shared<ParamsMessage>(name, params, target, origin);
}

shared_ptr<JsonMessage> MessageHelper::MakeJsonMessage(const string &name, const json &json, const string &target, const string &origin)
{
    return make_shared<JsonMessage>(name, json, target, origin);
}
