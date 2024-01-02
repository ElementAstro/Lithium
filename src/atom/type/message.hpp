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
#include "args.hpp"

// Base class Message
class Message
{
public:
    virtual ~Message() = default;

    // Message type enumeration
    enum class Type
    {
        kVoid,
        kText,
        kNumber,
        kBoolean,
        kAny,
        kParams,
        kMaxType
    };

    static Type fromInt(const int &t);

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

// VoidMessage class
// This is for the command that does not need any parameters
class VoidMessage : public Message
{
public:
    explicit VoidMessage(const std::string &name, const std::string &target, const std::string &origin);
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
    explicit ParamsMessage(const std::string &name, const Args &params, const std::string &target, const std::string &origin);

    Args value() const;

private:
    Args params_;
};

/**
 * @brief Message helper
 * @note This class is used to make messages
*/
class MessageHelper
{
public:
    /**
     * @brief Make a message
     * @param name Message name
     * @param target Message target
     * @param origin Message origin
     * @return Message
     * @note The message type is kVoid, which is used to send a command without parameters
     */
    static std::shared_ptr<VoidMessage> MakeVoidMessage(const std::string &name, const std::string &target, const std::string &origin);

    /**
     * @brief Make a message
     * @param name Message name
     * @param value Message value
     * @param target Message target
     * @param origin Message origin
     * @return Message
     * @note The message type is kText, which is used to send a command with a string parameter
     */
    static std::shared_ptr<TextMessage> MakeTextMessage(const std::string &name, const std::string &value, const std::string &target, const std::string &origin);

    /**
     * @brief Make a message
     * @param name Message name
     * @param value Message value
     * @param target Message target
     * @param origin Message origin
     * @return Message
     * @note The message type is kNumber, which is used to send a command with a number parameter
     */
    static std::shared_ptr<NumberMessage> MakeNumberMessage(const std::string &name, double value, const std::string &target, const std::string &origin);

    /**
     * @brief Make a message
     * @param name Message name
     * @param value Message value
     * @param target Message target
     * @param origin Message origin
     * @return Message
     * @note The message type is kBoolean, which is used to send a command with a boolean parameter
     */
    static std::shared_ptr<BooleanMessage> MakeBooleanMessage(const std::string &name, bool value, const std::string &target, const std::string &origin);

    /**
     * @brief Make a message
     * @param name Message name
     * @param data Message value
     * @param target Message target
     * @param origin Message origin
     * @return Message
     * @note The message type is kAny, which is used to send a command with a any parameter
     * @note This is a little bit dangerous, please use it carefully
     * @note std::bad_any_cast exception will be thrown if the type of data is not the same as the type of the parameter
     */
    static std::shared_ptr<AnyMessage> MakeAnyMessage(const std::string &name, const std::any &value, const std::string &target, const std::string &origin);

    /**
     * @brief Make a message
     * @param name Message name
     * @param value Message value
     * @param target Message target
     * @param origin Message origin
     * @return Message
     * @note The message type is kParams, which is used to send a command with a Args parameter
     */
    static std::shared_ptr<ParamsMessage> MakeParamsMessage(const std::string &name, const Args &value, const std::string &target, const std::string &origin);
};

using ReturnMessage = std::variant<std::shared_ptr<TextMessage>,
                                   std::shared_ptr<NumberMessage>,
                                   std::shared_ptr<BooleanMessage>,
                                   std::shared_ptr<AnyMessage>,
                                   std::shared_ptr<ParamsMessage>>;