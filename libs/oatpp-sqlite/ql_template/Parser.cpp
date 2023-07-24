/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
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

#include "Parser.hpp"

#include "oatpp/core/data/stream/BufferStream.hpp"
#include "oatpp/core/parser/ParsingError.hpp"

namespace oatpp { namespace sqlite { namespace ql_template {

oatpp::String Parser::preprocess(const oatpp::String& text, std::vector<CleanSection>& cleanSections) {

  data::stream::BufferOutputStream ss;
  parser::Caret caret(text);

  bool writeChar = true;

  v_buff_size sectionStart = -1;

  while(caret.canContinue()) {

    v_char8 c = *caret.getCurrData();
    writeChar = true;

    switch(c) {

      case '\'': {
        auto l = caret.putLabel();
        skipStringInQuotes(caret);
        ss.writeSimple(l.getData(), l.getSize());
        writeChar = false;
        break;
      }
      case '$': {
        auto l = caret.putLabel();
        skipStringInDollars(caret);
        ss.writeSimple(l.getData(), l.getSize());
        writeChar = false;
        break;
      }

      case '<': {
        if(sectionStart == -1) {
          if(caret.isAtText("<!!", 3, true)) {
            sectionStart = ss.getCurrentPosition();
            writeChar = false;
          } else {
            caret.inc();
          }
        } else {
          caret.inc();
        }
        break;
      }

      case '!': {
        if(sectionStart != -1) {
          if(caret.isAtText("!!>", 3, true)) {
            cleanSections.emplace_back(CleanSection(sectionStart, ss.getCurrentPosition() - sectionStart));
            sectionStart = -1;
            writeChar = false;
          } else {
            caret.inc();
          }
        } else {
          caret.inc();
        }
        break;
      }

      default:
        caret.inc();

    }

    if(writeChar) {
      ss.writeCharSimple(c);
    }

  }

  return ss.toString();

}

data::share::StringTemplate::Variable Parser::parseIdentifier(parser::Caret& caret) {
  data::share::StringTemplate::Variable result;
  result.posStart = caret.getPosition();
  if(caret.canContinueAtChar(':', 1)) {
    auto label = caret.putLabel();
    while(caret.canContinue()) {
      v_char8 a = *caret.getCurrData();
      bool isAllowedChar = (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || (a >= '0' && a <= '9') || (a == '_') || (a == '.');
      if(!isAllowedChar) {
        result.posEnd = caret.getPosition() - 1;
        result.name = label.toString();
        return result;
      }
      caret.inc();
    }
    result.name = label.toString();
  } else {
    caret.setError("Invalid identifier");
  }
  result.posEnd = caret.getPosition() - 1;
  return result;
}

void Parser::skipStringInQuotes(parser::Caret& caret) {

  bool opened = false;
  while(caret.canContinueAtChar('\'', 1)) {
    opened = true;
    if(caret.findChar('\'')) {
      caret.inc();
      opened = false;
    }
  }

  if(opened) {
    caret.setError("Invalid quote-enclosed string");
  }

}

void Parser::skipStringInDollars(parser::Caret& caret) {

  if(caret.canContinueAtChar('$', 1)) {

    auto label = caret.putLabel();
    if(!caret.findChar('$')) {
      caret.setError("Invalid dollar-enclosed string");
      return;
    }
    caret.inc();
    auto term = label.toString();

    while(caret.canContinue()) {

      if(caret.findChar('$')) {
        caret.inc();
        if(caret.isAtText(term->data(), term->size(), true)) {
          return;
        }
      }

    }

  }

  caret.setError("Invalid dollar-enclosed string");

}

data::share::StringTemplate Parser::parseTemplate(const oatpp::String& text) {

  std::vector<CleanSection> cleanSections;
  auto processedText = preprocess(text, cleanSections);

  parser::Caret caret(processedText);

  std::vector<data::share::StringTemplate::Variable> variables;

  v_buff_size currSection = 0;

  while(caret.canContinue()) {

    if(currSection < cleanSections.size() && cleanSections[currSection].position == caret.getPosition()) {
      caret.inc(cleanSections[currSection].size);
      currSection ++;
      continue;
    }

    v_char8 c = *caret.getCurrData();

    switch(c) {

      case ':': {
        auto var = parseIdentifier(caret);
        if(var.name) {
          variables.push_back(var);
        }
      }
        break;

      case '\'': skipStringInQuotes(caret); break;
      case '$': skipStringInDollars(caret); break;

      default:
        caret.inc();

    }

  }

  if(caret.hasError()) {
    throw oatpp::parser::ParsingError(caret.getErrorMessage(), caret.getErrorCode(), caret.getPosition());
  }

  return data::share::StringTemplate(processedText, std::move(variables));

}

}}}
