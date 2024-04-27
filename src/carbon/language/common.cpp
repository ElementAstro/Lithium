#include "common.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <unordered_set>
#include "../command/boxed_value.hpp"
#include "../command/dispatchkit.hpp"
#include "../command/proxy_functions.hpp"
#include "../defines.hpp"
#include "atom/experiment/type_info.hpp"

namespace Carbon {
Parse_Location::Parse_Location(std::string t_fname, const int t_start_line,
                               const int t_start_col, const int t_end_line,
                               const int t_end_col)
    : start(t_start_line, t_start_col),
      end(t_end_line, t_end_col),
      filename(std::make_shared<std::string>(std::move(t_fname))) {}

Parse_Location::Parse_Location(std::shared_ptr<std::string> t_fname,
                               const int t_start_line, const int t_start_col,
                               const int t_end_line, const int t_end_col)
    : start(t_start_line, t_start_col),
      end(t_end_line, t_end_col),
      filename(std::move(t_fname)) {}

const std::string &AST_Node::filename() const noexcept {
    return *location.filename;
}

const File_Position &AST_Node::start() const noexcept { return location.start; }

const File_Position &AST_Node::end() const noexcept { return location.end; }

std::string AST_Node::pretty_print() const {
    std::ostringstream oss;

    oss << text;

    for (auto &elem : get_children()) {
        oss << elem.get().pretty_print() << ' ';
    }

    return oss.str();
}

/// Prints the contents of an AST node, including its children, recursively
std::string AST_Node::to_string(const std::string &t_prepend) const {
    std::ostringstream oss;

    oss << t_prepend << "(" << ast_node_type_to_string(this->identifier) << ") "
        << this->text << " : " << this->location.start.line << ", "
        << this->location.start.column << '\n';

    for (auto &elem : get_children()) {
        oss << elem.get().to_string(t_prepend + "  ");
    }
    return oss.str();
}

AST_Node::AST_Node(std::string t_ast_node_text, AST_Node_Type t_id,
                   Parse_Location t_loc)
    : identifier(t_id),
      text(std::move(t_ast_node_text)),
      location(std::move(t_loc)) {}

const std::string &AST_Node_Trace::filename() const noexcept {
    return *location.filename;
}

const File_Position &AST_Node_Trace::start() const noexcept {
    return location.start;
}

const File_Position &AST_Node_Trace::end() const noexcept {
    return location.end;
}

std::string AST_Node_Trace::pretty_print() const {
    std::ostringstream oss;

    oss << text;

    for (const auto &elem : children) {
        oss << elem.pretty_print() << ' ';
    }

    return oss.str();
}

std::vector<AST_Node_Trace> AST_Node_Trace::get_children(const AST_Node &node) {
    const auto node_children = node.get_children();
    return std::vector<AST_Node_Trace>(node_children.begin(),
                                       node_children.end());
}

AST_Node_Trace::AST_Node_Trace(const AST_Node &node)
    : identifier(node.identifier),
      text(node.text),
      location(node.location),
      children(get_children(node)) {}

/// \brief Classes which may be thrown during error cases when ChaiScript is
/// executing.
namespace exception {
load_module_error::load_module_error(const std::string &t_reason)
    : std::runtime_error(t_reason) {}

load_module_error::load_module_error(
    const std::string &t_name, const std::vector<load_module_error> &t_errors)
    : std::runtime_error(format_error(t_name, t_errors)) {}

std::string load_module_error::format_error(
    const std::string &t_name, const std::vector<load_module_error> &t_errors) {
    std::stringstream ss;
    ss << "Error loading module '" << t_name << "'\n"
       << "  The following locations were searched:\n";

    for (const auto &err : t_errors) {
        ss << "    " << err.what() << "\n";
    }

    return ss.str();
}

eval_error::eval_error(
    const std::string &t_why, const File_Position &t_where,
    const std::string &t_fname, const std::vector<Boxed_Value> &t_parameters,
    const std::vector<Carbon::Const_Proxy_Function> &t_functions,
    bool t_dot_notation, const Carbon::detail::Dispatch_Engine &t_ss) noexcept
    : std::runtime_error(
          format(t_why, t_where, t_fname, t_parameters, t_dot_notation, t_ss)),
      reason(t_why),
      start_position(t_where),
      filename(t_fname),
      detail(format_detail(t_functions, t_dot_notation, t_ss)) {}

eval_error::eval_error(
    const std::string &t_why, const std::vector<Boxed_Value> &t_parameters,
    const std::vector<Carbon::Const_Proxy_Function> &t_functions,
    bool t_dot_notation, const Carbon::detail::Dispatch_Engine &t_ss) noexcept
    : std::runtime_error(format(t_why, t_parameters, t_dot_notation, t_ss)),
      reason(t_why),
      detail(format_detail(t_functions, t_dot_notation, t_ss)) {}

eval_error::eval_error(const std::string &t_why, const File_Position &t_where,
                       const std::string &t_fname) noexcept
    : std::runtime_error(format(t_why, t_where, t_fname)),
      reason(t_why),
      start_position(t_where),
      filename(t_fname) {}

eval_error::eval_error(const std::string &t_why) noexcept
    : std::runtime_error("Error: \"" + t_why + "\" "), reason(t_why) {}

std::string eval_error::pretty_print() const {
    std::ostringstream ss;

    ss << what();
    if (!call_stack.empty()) {
        ss << "during evaluation at (" << fname(call_stack[0]) << " "
           << startpos(call_stack[0]) << ")\n";
        ss << '\n' << detail << '\n';
        ss << "  " << fname(call_stack[0]) << " (" << startpos(call_stack[0])
           << ") '" << pretty(call_stack[0]) << "'";
        for (size_t j = 1; j < call_stack.size(); ++j) {
            if (id(call_stack[j]) != Carbon::AST_Node_Type::Block &&
                id(call_stack[j]) != Carbon::AST_Node_Type::File) {
                ss << '\n';
                ss << "  from " << fname(call_stack[j]) << " ("
                   << startpos(call_stack[j]) << ") '" << pretty(call_stack[j])
                   << "'";
            }
        }
    }
    ss << '\n';
    return ss.str();
}

std::string eval_error::format_why(const std::string &t_why) {
    return "Error: \"" + t_why + "\"";
}

std::string eval_error::format_types(
    const Const_Proxy_Function &t_func, bool t_dot_notation,
    const Carbon::detail::Dispatch_Engine &t_ss) {
    assert(t_func);
    int arity = t_func->get_arity();
    std::vector<Type_Info> types = t_func->get_param_types();

    std::string retval;
    if (arity == -1) {
        retval = "(...)";
        if (t_dot_notation) {
            retval = "(Object)." + retval;
        }
    } else if (types.size() <= 1) {
        retval = "()";
    } else {
        std::stringstream ss;
        ss << "(";

        std::string paramstr;

        for (size_t index = 1; index != types.size(); ++index) {
            paramstr += (types[index].is_const() ? "const " : "");
            paramstr += t_ss.get_type_name(types[index]);

            if (index == 1 && t_dot_notation) {
                paramstr += ").(";
                if (types.size() == 2) {
                    paramstr += ", ";
                }
            } else {
                paramstr += ", ";
            }
        }

        ss << paramstr.substr(0, paramstr.size() - 2);

        ss << ")";
        retval = ss.str();
    }

    std::shared_ptr<const dispatch::Dynamic_Proxy_Function> dynfun =
        std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(
            t_func);

    if (dynfun && dynfun->has_parse_tree()) {
        Proxy_Function f = dynfun->get_guard();

        if (f) {
            auto dynfunguard = std::dynamic_pointer_cast<
                const dispatch::Dynamic_Proxy_Function>(f);
            if (dynfunguard && dynfunguard->has_parse_tree()) {
                retval += " : " + format_guard(dynfunguard->get_parse_tree());
            }
        }

        retval += "\n          Defined at " +
                  format_location(dynfun->get_parse_tree());
    }

    return retval;
}

std::string eval_error::format_detail(
    const std::vector<Carbon::Const_Proxy_Function> &t_functions,
    bool t_dot_notation, const Carbon::detail::Dispatch_Engine &t_ss) {
    std::stringstream ss;
    if (t_functions.size() == 1) {
        assert(t_functions[0]);
        ss << "  Expected: "
           << format_types(t_functions[0], t_dot_notation, t_ss) << '\n';
    } else {
        ss << "  " << t_functions.size() << " overloads available:\n";

        for (const auto &t_function : t_functions) {
            ss << "      " << format_types((t_function), t_dot_notation, t_ss)
               << '\n';
        }
    }

    return ss.str();
}

std::string eval_error::format_parameters(
    const std::vector<Boxed_Value> &t_parameters, bool t_dot_notation,
    const Carbon::detail::Dispatch_Engine &t_ss) {
    std::stringstream ss;
    ss << "(";

    if (!t_parameters.empty()) {
        std::string paramstr;

        for (auto itr = t_parameters.begin(); itr != t_parameters.end();
             ++itr) {
            paramstr += (itr->is_const() ? "const " : "");
            paramstr += t_ss.type_name(*itr);

            if (itr == t_parameters.begin() && t_dot_notation) {
                paramstr += ").(";
                if (t_parameters.size() == 1) {
                    paramstr += ", ";
                }
            } else {
                paramstr += ", ";
            }
        }

        ss << paramstr.substr(0, paramstr.size() - 2);
    }
    ss << ")";

    return ss.str();
}

std::string eval_error::format_filename(const std::string &t_fname) {
    std::stringstream ss;

    if (t_fname != "__EVAL__") {
        ss << "in '" << t_fname << "' ";
    } else {
        ss << "during evaluation ";
    }

    return ss.str();
}

std::string eval_error::format_location(const File_Position &t_where) {
    std::stringstream ss;
    ss << "at (" << t_where.line << ", " << t_where.column << ")";
    return ss.str();
}

std::string eval_error::format(const std::string &t_why,
                               const File_Position &t_where,
                               const std::string &t_fname,
                               const std::vector<Boxed_Value> &t_parameters,
                               bool t_dot_notation,
                               const Carbon::detail::Dispatch_Engine &t_ss) {
    std::stringstream ss;

    ss << format_why(t_why);
    ss << " ";

    ss << "With parameters: "
       << format_parameters(t_parameters, t_dot_notation, t_ss);
    ss << " ";

    ss << format_filename(t_fname);
    ss << " ";

    ss << format_location(t_where);

    return ss.str();
}

std::string eval_error::format(const std::string &t_why,
                               const std::vector<Boxed_Value> &t_parameters,
                               bool t_dot_notation,
                               const Carbon::detail::Dispatch_Engine &t_ss) {
    std::stringstream ss;

    ss << format_why(t_why);
    ss << " ";

    ss << "With parameters: "
       << format_parameters(t_parameters, t_dot_notation, t_ss);
    ss << " ";

    return ss.str();
}

std::string eval_error::format(const std::string &t_why,
                               const File_Position &t_where,
                               const std::string &t_fname) {
    std::stringstream ss;

    ss << format_why(t_why);
    ss << " ";

    ss << format_filename(t_fname);
    ss << " ";

    ss << format_location(t_where);

    return ss.str();
}

file_not_found_error::file_not_found_error(const std::string &t_filename)
    : std::runtime_error("File Not Found: " + t_filename),
      filename(t_filename) {}

}  // namespace exception

// static
bool AST_Node::get_bool_condition(const Boxed_Value &t_bv,
                                  const Carbon::detail::Dispatch_State &t_ss) {
    try {
        return t_ss->boxed_cast<bool>(t_bv);
    } catch (const exception::bad_boxed_cast &) {
        throw exception::eval_error("Condition not boolean");
    }
}

namespace eval {
namespace detail {

Scope_Push_Pop::Scope_Push_Pop(const Carbon::detail::Dispatch_State &t_ds)
    : m_ds(t_ds) {
    m_ds->new_scope(m_ds.stack_holder());
}

Scope_Push_Pop::~Scope_Push_Pop() { m_ds->pop_scope(m_ds.stack_holder()); }

Function_Push_Pop::Function_Push_Pop(const Carbon::detail::Dispatch_State &t_ds)
    : m_ds(t_ds) {
    m_ds->new_function_call(m_ds.stack_holder(), m_ds.conversion_saves());
}

Function_Push_Pop::~Function_Push_Pop() {
    m_ds->pop_function_call(m_ds.stack_holder(), m_ds.conversion_saves());
}

void Function_Push_Pop::save_params(const Function_Params &t_params) {
    m_ds->save_function_params(t_params);
}

Stack_Push_Pop::Stack_Push_Pop(const Carbon::detail::Dispatch_State &t_ds)
    : m_ds(t_ds) {
    m_ds->new_stack(m_ds.stack_holder());
}

Stack_Push_Pop::~Stack_Push_Pop() { m_ds->pop_stack(m_ds.stack_holder()); }

}  // namespace detail
}  // namespace eval
}  // namespace Carbon
