/*
 * message.hpp
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

#pragma once

#include <string>
#include <any>
#include <memory>
#include "atom/type/json.hpp"

using json = nlohmann::json;

class IParams;

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
        kAny,
        kParams,
        kJson,
        kMaxType
    };

    Type type() const;
    std::string_view target() const;
    std::string_view origin() const;
    std::string timestamp() const;
    std::string name() const;
    double api_version() const;

protected:
    explicit Message(Type t, const std::string &name, const std::string &target, const std::string &origin);

private:
    Type type_;
    std::string target_;
    std::string origin_;
    std::string name_;
    std::string timestamp_;
    std::string uuid_;
    double api_version_{1.0};
};

// TextMessage class
class TextMessage : public Message
{
public:
    explicit TextMessage(const std::string &name, const std::string &text, const std::string &target, const std::string &origin);

    std::string value() const;

private:
    std::string value_;
};

// NumberMessage class
class NumberMessage : public Message
{
public:
    explicit NumberMessage(const std::string &name, double number, const std::string &target, const std::string &origin);

    double value() const;

private:
    double value_;
};

class BooleanMessage : public Message
{
public:
    explicit BooleanMessage(const std::string &name, bool value, const std::string &target, const std::string &origin);

    bool value() const;

private:
    bool value_;
};

class AnyMessage : public Message
{
public:
    explicit AnyMessage(const std::string &name, const std::any &data, const std::string &target, const std::string &origin);

    std::any value() const;
    std::string type() const;

private:
    std::any data_;
    std::string type_;
};

class ParamsMessage : public Message
{
public:
    explicit ParamsMessage(const std::string &name, std::shared_ptr<IParams> params, const std::string &target, const std::string &origin);

    std::shared_ptr<IParams> value() const;

private:
    std::shared_ptr<IParams> params_;
};

class JsonMessage : public Message
{
public:
    explicit JsonMessage(const std::string &name, const json &json, const std::string &target, const std::string &origin);

    json value() const;

private:
    json value_;
};

class MessageHelper
{
public:
    static std::shared_ptr<TextMessage> MakeTextMessage(const std::string &name, const std::string &value, const std::string &target, const std::string &origin);

    static std::shared_ptr<NumberMessage> MakeNumberMessage(const std::string &name, double value, const std::string &target, const std::string &origin);

    static std::shared_ptr<BooleanMessage> MakeBooleanMessage(const std::string &name, bool value, const std::string &target, const std::string &origin);

    static std::shared_ptr<AnyMessage> MakeAnyMessage(const std::string &name, const std::any &data, const std::string &target, const std::string &origin);

    static std::shared_ptr<ParamsMessage> MakeParamsMessage(const std::string &name, std::shared_ptr<IParams> params, const std::string &target, const std::string &origin);

    static std::shared_ptr<JsonMessage> MakeJsonMessage(const std::string &name, const json &json, const std::string &target, const std::string &origin);
};

using ReturnMessage = std::variant<std::shared_ptr<TextMessage>,
                                 std::shared_ptr<NumberMessage>,
                                 std::shared_ptr<BooleanMessage>,
                                 std::shared_ptr<JsonMessage>,
                                 std::shared_ptr<AnyMessage>,
                                 std::shared_ptr<ParamsMessage>>;