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

#ifndef oatpp_sqlite_ql_template_Parser_hpp
#define oatpp_sqlite_ql_template_Parser_hpp

#include "oatpp/core/data/share/StringTemplate.hpp"
#include "oatpp/core/parser/Caret.hpp"

#include <sqlite3.h>

namespace oatpp { namespace sqlite { namespace ql_template {

/**
 * Query template parser.
 */
class Parser {
public:

  /**
   * Template extra info.
   */
  struct TemplateExtra {

    /**
     * Template name.
     */
    oatpp::String templateName;

    /**
     * Template text with parameters substituted to SQLite parameter placeholders.
     */
    oatpp::String preparedTemplate;

    /**
     * Use prepared statement for this query.
     */
    bool prepare;
  };

public:

  struct CleanSection {
    CleanSection(v_buff_size p, v_buff_size s)
      : position(p)
      , size(s)
    {}
    v_buff_size position;
    v_buff_size size;
  };

private:
  static data::share::StringTemplate::Variable parseIdentifier(parser::Caret& caret);
  static void skipStringInQuotes(parser::Caret& caret);
  static void skipStringInDollars(parser::Caret& caret);
public:

  /**
   * Preprocess text.
   * @param text
   * @param cleanSections - out vector of clean sections.
   * @return
   */
  static oatpp::String preprocess(const oatpp::String& text, std::vector<CleanSection>& cleanSections);

  /**
   * Parse query template.
   * @param text
   * @return - &id:oatpp::data::share::StringTemplate;.
   */
  static data::share::StringTemplate parseTemplate(const oatpp::String& text);

};

}}}

#endif // oatpp_sqlite_ql_template_Parser_hpp
