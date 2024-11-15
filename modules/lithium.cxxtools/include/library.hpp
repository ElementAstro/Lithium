// library.hpp
#ifndef LIBRARY_HPP
#define LIBRARY_HPP

#include "ini2json.hpp"
#include "csv2json.hpp"
#include "yaml2json.hpp"
#include "xml2json.hpp"

namespace lithium::cxxtools {

using Ini2Json = detail::Ini2Json;
using Csv2Json = detail::Csv2Json;
using Yaml2Json = detail::Yaml2Json;
using Xml2Json = detail::Xml2Json;

} // namespace lithium::cxxtools

#endif // LIBRARY_HPP