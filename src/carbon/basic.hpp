

#ifndef CARBON_BASIC_HPP
#define CARBON_BASIC_HPP

#include "defines.hpp"

#include "command/boxed_number.hpp"
#include "command/dispatchkit.hpp"
#include "command/dynamic_object.hpp"
#include "command/function_call.hpp"

#include "language/engine.hpp"
#include "language/eval.hpp"

// This file includes all of the basic requirements for ChaiScript,
// to use, you might do something like:
//

/*

#include "language/parser.hpp"
#include "stdlib.hpp"

Carbon_Basic chai(
          Carbon::Std_Lib::library(),
          std::make_unique<parser::Carbon_Parser<eval::Noop_Tracer,
optimizer::Optimizer_Default>>());

*/

// If you want a fully packaged ready to go ChaiScript, use chaiscript.hpp

#endif /* CARBON_BASIC_HPP */
