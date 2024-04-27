

#ifndef CARBON_COMMON_HPP
#define CARBON_COMMON_HPP

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
struct AST_Node;
struct AST_Node_Trace;
namespace exception {
struct eval_error;
}
}  // namespace Carbon

namespace Carbon {
struct Name_Validator {
    template <typename T>
    static bool is_reserved_word(const T &s) noexcept {
        const static std::unordered_set<std::uint32_t> words{
            Atom::Algorithm::fnv1a_hash("def"),
            Atom::Algorithm::fnv1a_hash("fun"),
            Atom::Algorithm::fnv1a_hash("while"),
            Atom::Algorithm::fnv1a_hash("for"),
            Atom::Algorithm::fnv1a_hash("if"),
            Atom::Algorithm::fnv1a_hash("else"),
            Atom::Algorithm::fnv1a_hash("&&"),
            Atom::Algorithm::fnv1a_hash("||"),
            Atom::Algorithm::fnv1a_hash(","),
            Atom::Algorithm::fnv1a_hash("auto"),
            Atom::Algorithm::fnv1a_hash("return"),
            Atom::Algorithm::fnv1a_hash("break"),
            Atom::Algorithm::fnv1a_hash("true"),
            Atom::Algorithm::fnv1a_hash("false"),
            Atom::Algorithm::fnv1a_hash("class"),
            Atom::Algorithm::fnv1a_hash("attr"),
            Atom::Algorithm::fnv1a_hash("var"),
            Atom::Algorithm::fnv1a_hash("global"),
            Atom::Algorithm::fnv1a_hash("GLOBAL"),
            Atom::Algorithm::fnv1a_hash("_"),
            Atom::Algorithm::fnv1a_hash("__LINE__"),
            Atom::Algorithm::fnv1a_hash("__FILE__"),
            Atom::Algorithm::fnv1a_hash("__FUNC__"),
            Atom::Algorithm::fnv1a_hash("__CLASS__")};

        return words.count(Atom::Algorithm::fnv1a_hash(s)) == 1;
    }

    template <typename T>
    static bool valid_object_name(const T &name) noexcept {
        return name.find("::") == std::string::npos && !is_reserved_word(name);
    }

    template <typename T>
    static void validate_object_name(const T &name) {
        if (is_reserved_word(name)) {
            throw exception::reserved_word_error(std::string(name));
        }

        if (name.find("::") != std::string::npos) {
            throw exception::illegal_name_error(std::string(name));
        }
    }
};

/// Signature of module entry point that all binary loadable modules must
/// implement.
using Create_Module_Func = ModulePtr (*)();

/// Types of AST nodes available to the parser and eval
enum class AST_Node_Type {
    Id,
    Fun_Call,
    Unused_Return_Fun_Call,
    Arg_List,
    Equation,
    Var_Decl,
    Assign_Decl,
    Array_Call,
    Dot_Access,
    Lambda,
    Block,
    Scopeless_Block,
    Def,
    While,
    If,
    For,
    Ranged_For,
    Inline_Array,
    Inline_Map,
    Return,
    File,
    Prefix,
    Break,
    Continue,
    Map_Pair,
    Value_Range,
    Inline_Range,
    Try,
    Catch,
    Finally,
    Method,
    Attr_Decl,
    Logical_And,
    Logical_Or,
    Reference,
    Switch,
    Case,
    Default,
    Noop,
    Class,
    Binary,
    Arg,
    Global_Decl,
    Constant,
    Compiled
};

enum class Operator_Precedence {
    Ternary_Cond,
    Logical_Or,
    Logical_And,
    Bitwise_Or,
    Bitwise_Xor,
    Bitwise_And,
    Equality,
    Comparison,
    Shift,
    Addition,
    Multiplication,
    Prefix
};

namespace {
/// Helper lookup to get the name of each node type
constexpr const char *ast_node_type_to_string(
    AST_Node_Type ast_node_type) noexcept {
    constexpr const char *const ast_node_types[] = {
        "Id",          "Fun_Call",    "Unused_Return_Fun_Call",
        "Arg_List",    "Equation",    "Var_Decl",
        "Assign_Decl", "Array_Call",  "Dot_Access",
        "Lambda",      "Block",       "Scopeless_Block",
        "Def",         "While",       "If",
        "For",         "Ranged_For",  "Inline_Array",
        "Inline_Map",  "Return",      "File",
        "Prefix",      "Break",       "Continue",
        "Map_Pair",    "Value_Range", "Inline_Range",
        "Try",         "Catch",       "Finally",
        "Method",      "Attr_Decl",   "Logical_And",
        "Logical_Or",  "Reference",   "Switch",
        "Case",        "Default",     "Noop",
        "Class",       "Binary",      "Arg",
        "Global_Decl", "Constant",    "Compiled"};

    return ast_node_types[static_cast<int>(ast_node_type)];
}
}  // namespace

/// \brief Convenience type for file positions
struct File_Position {
    int line = 0;
    int column = 0;

    constexpr File_Position(int t_file_line, int t_file_column) noexcept
        : line(t_file_line), column(t_file_column) {}

    constexpr File_Position() noexcept = default;
};

struct Parse_Location {
    Parse_Location(std::string t_fname = "", const int t_start_line = 0,
                   const int t_start_col = 0, const int t_end_line = 0,
                   const int t_end_col = 0);

    Parse_Location(std::shared_ptr<std::string> t_fname,
                   const int t_start_line = 0, const int t_start_col = 0,
                   const int t_end_line = 0, const int t_end_col = 0);

    File_Position start;
    File_Position end;
    std::shared_ptr<std::string> filename;
};

/// \brief Struct that doubles as both a parser ast_node and an AST node.
struct AST_Node {
public:
    const AST_Node_Type identifier;
    const std::string text;
    Parse_Location location;

    const std::string &filename() const noexcept;

    const File_Position &start() const noexcept;

    const File_Position &end() const noexcept;

    std::string pretty_print() const;

    virtual std::vector<std::reference_wrapper<AST_Node>> get_children()
        const = 0;
    virtual Boxed_Value eval(
        const Carbon::detail::Dispatch_State &t_e) const = 0;

    /// Prints the contents of an AST node, including its children, recursively
    std::string to_string(const std::string &t_prepend = "") const;

    static bool get_bool_condition(
        const Boxed_Value &t_bv, const Carbon::detail::Dispatch_State &t_ss);

    virtual ~AST_Node() noexcept = default;
    AST_Node(AST_Node &&) = default;
    AST_Node &operator=(AST_Node &&) = delete;
    AST_Node(const AST_Node &) = delete;
    AST_Node &operator=(const AST_Node &) = delete;

protected:
    AST_Node(std::string t_ast_node_text, AST_Node_Type t_id,
             Parse_Location t_loc);
};

/// \brief Typedef for pointers to AST_Node objects. Used in building of the
/// AST_Node tree
using AST_NodePtr = std::unique_ptr<AST_Node>;
using AST_NodePtr_Const = std::unique_ptr<const AST_Node>;

struct AST_Node_Trace {
    const AST_Node_Type identifier;
    const std::string text;
    Parse_Location location;

    const std::string &filename() const noexcept;

    const File_Position &start() const noexcept;

    const File_Position &end() const noexcept;

    std::string pretty_print() const;

    std::vector<AST_Node_Trace> get_children(const AST_Node &node);

    AST_Node_Trace(const AST_Node &node);

    std::vector<AST_Node_Trace> children;
};

/// \brief Classes which may be thrown during error cases when ChaiScript is
/// executing.
namespace exception {
/// \brief Thrown if an error occurs while attempting to load a binary module
struct load_module_error : std::runtime_error {
    explicit load_module_error(const std::string &t_reason);

    load_module_error(const std::string &t_name,
                      const std::vector<load_module_error> &t_errors);

    load_module_error(const load_module_error &) = default;
    ~load_module_error() noexcept override = default;

    static std::string format_error(
        const std::string &t_name,
        const std::vector<load_module_error> &t_errors);
};

/// Errors generated during parsing or evaluation
struct eval_error : std::runtime_error {
    std::string reason;
    File_Position start_position;
    std::string filename;
    std::string detail;
    std::vector<AST_Node_Trace> call_stack;

    eval_error(const std::string &t_why, const File_Position &t_where,
               const std::string &t_fname,
               const std::vector<Boxed_Value> &t_parameters,
               const std::vector<Carbon::Const_Proxy_Function> &t_functions,
               bool t_dot_notation,
               const Carbon::detail::Dispatch_Engine &t_ss) noexcept;

    eval_error(const std::string &t_why,
               const std::vector<Boxed_Value> &t_parameters,
               const std::vector<Carbon::Const_Proxy_Function> &t_functions,
               bool t_dot_notation,
               const Carbon::detail::Dispatch_Engine &t_ss) noexcept;

    eval_error(const std::string &t_why, const File_Position &t_where,
               const std::string &t_fname) noexcept;

    explicit eval_error(const std::string &t_why) noexcept;

    eval_error(const eval_error &) = default;

    std::string pretty_print() const;

    ~eval_error() noexcept override = default;

private:
    template <typename T>
    static AST_Node_Type id(const T &t) noexcept {
        return t.identifier;
    }

    template <typename T>
    static std::string pretty(const T &t) {
        return t.pretty_print();
    }

    template <typename T>
    static const std::string &fname(const T &t) noexcept {
        return t.filename();
    }

    template <typename T>
    static std::string startpos(const T &t) {
        std::ostringstream oss;
        oss << t.start().line << ", " << t.start().column;
        return oss.str();
    }

    static std::string format_why(const std::string &t_why);

    static std::string format_types(
        const Const_Proxy_Function &t_func, bool t_dot_notation,
        const Carbon::detail::Dispatch_Engine &t_ss);

    template <typename T>
    static std::string format_guard(const T &t) {
        return t.pretty_print();
    }

    template <typename T>
    static std::string format_location(const T &t) {
        std::ostringstream oss;
        oss << "(" << t.filename() << " " << t.start().line << ", "
            << t.start().column << ")";
        return oss.str();
    }

    static std::string format_detail(
        const std::vector<Carbon::Const_Proxy_Function> &t_functions,
        bool t_dot_notation, const Carbon::detail::Dispatch_Engine &t_ss);

    static std::string format_parameters(
        const std::vector<Boxed_Value> &t_parameters, bool t_dot_notation,
        const Carbon::detail::Dispatch_Engine &t_ss);

    static std::string format_filename(const std::string &t_fname);

    static std::string format_location(const File_Position &t_where);

    static std::string format(const std::string &t_why,
                              const File_Position &t_where,
                              const std::string &t_fname,
                              const std::vector<Boxed_Value> &t_parameters,
                              bool t_dot_notation,
                              const Carbon::detail::Dispatch_Engine &t_ss);

    static std::string format(const std::string &t_why,
                              const std::vector<Boxed_Value> &t_parameters,
                              bool t_dot_notation,
                              const Carbon::detail::Dispatch_Engine &t_ss);

    static std::string format(const std::string &t_why,
                              const File_Position &t_where,
                              const std::string &t_fname);
};

/// Errors generated when loading a file
struct file_not_found_error : std::runtime_error {
    explicit file_not_found_error(const std::string &t_filename);

    file_not_found_error(const file_not_found_error &) = default;
    ~file_not_found_error() noexcept override = default;

    std::string filename;
};

}  // namespace exception

namespace parser {
class Carbon_Parser_Base {
public:
    virtual AST_NodePtr parse(const std::string &t_input,
                              const std::string &t_fname) = 0;
    virtual void debug_print(const AST_Node &t,
                             std::string prepend = "") const = 0;
    virtual void *get_tracer_ptr() = 0;
    virtual ~Carbon_Parser_Base() = default;
    Carbon_Parser_Base() = default;
    Carbon_Parser_Base(Carbon_Parser_Base &&) = default;
    Carbon_Parser_Base &operator=(Carbon_Parser_Base &&) = delete;
    Carbon_Parser_Base &operator=(const Carbon_Parser_Base &&) = delete;

    template <typename T>
    T &get_tracer() noexcept {
        // to do type check this somehow?
        return *static_cast<T *>(get_tracer_ptr());
    }

protected:
    Carbon_Parser_Base(const Carbon_Parser_Base &) = default;
};
}  // namespace parser

namespace eval {
namespace detail {
/// Special type for returned values
struct Return_Value {
    Boxed_Value retval;
};

/// Special type indicating a call to 'break'
struct Break_Loop {};

/// Special type indicating a call to 'continue'
struct Continue_Loop {};

/// Creates a new scope then pops it on destruction
struct Scope_Push_Pop {
    Scope_Push_Pop(Scope_Push_Pop &&) = default;
    Scope_Push_Pop &operator=(Scope_Push_Pop &&) = delete;
    Scope_Push_Pop(const Scope_Push_Pop &) = delete;
    Scope_Push_Pop &operator=(const Scope_Push_Pop &) = delete;

    explicit Scope_Push_Pop(const Carbon::detail::Dispatch_State &t_ds);

    ~Scope_Push_Pop();

private:
    const Carbon::detail::Dispatch_State &m_ds;
};

/// Creates a new function call and pops it on destruction
struct Function_Push_Pop {
    Function_Push_Pop(Function_Push_Pop &&) = default;
    Function_Push_Pop &operator=(Function_Push_Pop &&) = delete;
    Function_Push_Pop(const Function_Push_Pop &) = delete;
    Function_Push_Pop &operator=(const Function_Push_Pop &) = delete;

    explicit Function_Push_Pop(const Carbon::detail::Dispatch_State &t_ds);

    ~Function_Push_Pop();

    void save_params(const Function_Params &t_params);

private:
    const Carbon::detail::Dispatch_State &m_ds;
};

/// Creates a new scope then pops it on destruction
struct Stack_Push_Pop {
    Stack_Push_Pop(Stack_Push_Pop &&) = default;
    Stack_Push_Pop &operator=(Stack_Push_Pop &&) = delete;
    Stack_Push_Pop(const Stack_Push_Pop &) = delete;
    Stack_Push_Pop &operator=(const Stack_Push_Pop &) = delete;

    explicit Stack_Push_Pop(const Carbon::detail::Dispatch_State &t_ds);

    ~Stack_Push_Pop();

private:
    const Carbon::detail::Dispatch_State &m_ds;
};
}  // namespace detail
}  // namespace eval
}  // namespace Carbon

#endif /* _CARBON_COMMON_HPP */
