/***************************************************************************
 *
 * Project:               _ _                 _
 *              /\  /\___| (_) ___ ___  _ __ | |_ ___ _ __
 *             / /_/ / _ \ | |/ __/ _ \| '_ \| __/ _ \ '__|
 *            / __  /  __/ | | (_| (_) | |_) | ||  __/ |
 *            \/ /_/ \___|_|_|\___\___/| .__/ \__\___|_|
 *                                     |_|
 *
 *
 * Copyright 2022-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * host <--> server <--> client
 * message codes.
 * - 0..99 general messages
 * - 100 - 199 outgoing host messages
 * - 200 - 299 incoming host messages
 * - 300 - 399 outgoing client messages
 * - 400 - 499 incoming client messages
 */
ENUM(MessageCodes, v_int32,

///////////////////////////////////////////////////////////////////
//// 0..99 general messages

     /**
      * Sent to a connection once connected.
      */
     VALUE(OUTGOING_HELLO, 0),

     /**
      * Server sends ping message to connection.
      */
     VALUE(OUTGOING_PING, 1),

     /**
      * Connection responds to server ping with pong message.
      */
     VALUE(INCOMING_PONG, 2),

     /**
      * Sent to connection to indicate operation error.
      */
     VALUE(OUTGOING_ERROR, 3),

     /**
      * Server notifies connections that new host has been elected.
      */
     VALUE(OUTGOING_NEW_HOST, 4),

     /**
      * Server sends message to connection from other connection.
      */
     VALUE(OUTGOING_MESSAGE, 5),

     /**
      * Connection broadcasts message to all clients.
      */
     VALUE(INCOMING_BROADCAST, 6),

     /**
      * Connection sends message to a client or to a group of clients.
      */
     VALUE(INCOMING_DIRECT_MESSAGE, 7),

     /**
      * Connection sends synchronized event. Synchronized event will be broadcasted to ALL connections, including the sender of this event.
      * All connections are guaranteed to receive synchronized events in the same order except for cases where connection's messages
      * were discarded due to poor connection (message queue overflow).
      */
     VALUE(INCOMING_SYNCHRONIZED_EVENT, 8),

     /**
      * Server send synchronized event to connection.
      */
     VALUE(OUTGOING_SYNCHRONIZED_EVENT, 9),

///////////////////////////////////////////////////////////////////
//// 100 - 199 outgoing host messages

     /**
      * Sent to host when new client joined the hub.
      */
     VALUE(OUTGOING_HOST_CLIENT_JOINED, 101),

     /**
      * Sent to host when client left the hub.
      */
     VALUE(OUTGOING_HOST_CLIENT_LEFT, 102),

///////////////////////////////////////////////////////////////////
//// 200 - 299 incoming host messages

     /**
      * Host sends to server to kick client or a group of clients.
      */
     VALUE(INCOMING_HOST_KICK_CLIENTS, 200),

///////////////////////////////////////////////////////////////////
//// 300 - 399 outgoing client messages

    /**
     * Client was kicked by a host.
     */
     VALUE(OUTGOING_CLIENT_KICKED, 300),

///////////////////////////////////////////////////////////////////
//// 400 - 499 incoming client messages

     /**
      * Client sends direct message to host.
      */
     VALUE(INCOMING_CLIENT_MESSAGE, 400)

);

/**
 * Error codes.
 */
ENUM(ErrorCodes, v_int32,

     /**
      * Request is malformed or it is missing required parameters.
      */
     VALUE(BAD_REQUEST, 0),

     /**
      * No hub config found on the server.
      */
     VALUE(GAME_NOT_FOUND, 1),

     /**
      * No hub session found for given sessionId.
      */
     VALUE(SESSION_NOT_FOUND, 2),

     /**
      * Operation not permitted.
      */
     VALUE(OPERATION_NOT_PERMITTED, 3),

     /**
      * Message is malformatted or violates configured restrictions.
      */
     VALUE(BAD_MESSAGE, 4),

     /**
      * Session is in an invalid state.
      */
     VALUE(INVALID_STATE, 5)

);

/**
 * Direct message
 */
class ErrorDto : public oatpp::DTO {

  DTO_INIT(ErrorDto, DTO)

  /**
   * Error code
   */
  DTO_FIELD(Enum<ErrorCodes>::AsNumber::NotNull, code);

  /**
   * Error text message
   */
  DTO_FIELD(String, message);

public:

  ErrorDto() = default;

  ErrorDto(const Enum<ErrorCodes>::AsNumber& pCode, const String pMessage)
    : code(pCode)
    , message(pMessage)
  {}

};

/**
 * Hello message.
 */
class HelloMessageDto : public oatpp::DTO {

  DTO_INIT(HelloMessageDto, DTO)

  /**
   * ID assigned to this connection by server.
   */
  DTO_FIELD(Int64, connectionId);

  /**
   * Message data
   */
  DTO_FIELD(Boolean, isHost);

};

/**
 * Direct message
 */
class DirectMessageDto : public oatpp::DTO {

  DTO_INIT(DirectMessageDto, DTO)

  /**
   * ConnectionIds of recipients
   */
  DTO_FIELD(Vector<Int64>, connectionIds) = {};

  /**
   * Message data
   */
  DTO_FIELD(String, data);

};

/**
 * Outgoing message.
 */
class OutgoingMessageDto : public oatpp::DTO {

  DTO_INIT(OutgoingMessageDto, DTO)

  /**
   * connectionId of sender
   */
  DTO_FIELD(Int64, connectionId);

  /**
   * Message data
   */
  DTO_FIELD(String, data);

};

/**
 * Outgoing synchronized message.
 */
class OutgoingSynchronizedMessageDto : public oatpp::DTO {

  DTO_INIT(OutgoingSynchronizedMessageDto, DTO)

  /**
   * Event Id - event index in the sequence.
   */
  DTO_FIELD(Int64, eventId);

  /**
   * connectionId of sender
   */
  DTO_FIELD(Int64, connectionId);

  /**
   * Message data
   */
  DTO_FIELD(String, data);

};

/**
 * Message
 */
class MessageDto : public oatpp::DTO {

  DTO_INIT(MessageDto, DTO)

  /**
   * Message code
   */
  DTO_FIELD(Enum<MessageCodes>::AsNumber::NotNull, code);

  /**
   * Operation Correlation ID
   */
  DTO_FIELD(oatpp::String, ocid);

  /**
   * Message payload
   */
  DTO_FIELD(oatpp::Any, payload);

  DTO_FIELD_TYPE_SELECTOR(payload) {

    if(!code) return Void::Class::getType();

    switch (*code) {

      case MessageCodes::OUTGOING_HELLO:
        return oatpp::Object<HelloMessageDto>::Class::getType();

      case MessageCodes::OUTGOING_PING:
        return oatpp::Int64::Class::getType();

      case MessageCodes::INCOMING_PONG:
        return oatpp::Int64::Class::getType();

      case MessageCodes::OUTGOING_ERROR:
        return oatpp::Object<ErrorDto>::Class::getType();

      case MessageCodes::OUTGOING_MESSAGE:
        return oatpp::Object<OutgoingMessageDto>::Class::getType();

      case MessageCodes::INCOMING_BROADCAST:
        return oatpp::String::Class::getType();

      case MessageCodes::INCOMING_DIRECT_MESSAGE:
        return oatpp::Object<DirectMessageDto>::Class::getType();

      case MessageCodes::INCOMING_SYNCHRONIZED_EVENT:
        return oatpp::String::Class::getType();

      case MessageCodes::OUTGOING_SYNCHRONIZED_EVENT:
        return oatpp::Object<OutgoingSynchronizedMessageDto>::Class::getType();

      case MessageCodes::OUTGOING_HOST_CLIENT_JOINED:
      case MessageCodes::OUTGOING_HOST_CLIENT_LEFT:
        return oatpp::Int64::Class::getType();

      case MessageCodes::INCOMING_HOST_KICK_CLIENTS:
        return oatpp::Vector<oatpp::Int64>::Class::getType();

      case MessageCodes::OUTGOING_CLIENT_KICKED:
        return oatpp::String::Class::getType();

      case MessageCodes::INCOMING_CLIENT_MESSAGE:
        return oatpp::String::Class::getType();

      default:
        throw std::runtime_error("not implemented");

    }

  }

public:

  MessageDto() = default;

  MessageDto(const Enum<MessageCodes>::AsNumber& pCode, const oatpp::Any& pPayload, const oatpp::String& pOCID = nullptr)
    : code(pCode)
    , payload(pPayload)
    , ocid(pOCID)
  {}

};


#include OATPP_CODEGEN_END(DTO)

#endif // DTOs_hpp
