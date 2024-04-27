#ifndef CARBON_HPP
#define CARBON_HPP

/// @mainpage
/// [CarbonScript](http://www.chaiscript.com") is a scripting language designed
/// specifically for integration with C++. It provides seamless integration with
/// C++ on all levels, including shared_ptr objects, functors and exceptions.
///
/// The parts of the CarbonScript API that the average user will be concerned
/// with are contained in the chaiscript namespace and the Carbon::CarbonScript
/// class.
///
/// The end user parts of the API are extremely simple both in size and ease of
/// use.
///
/// Currently, all source control and project management aspects of CarbonScript
/// occur on [github](http://www.github.com/CarbonScript/CarbonScript").
///
/// ------------------------------------------------------------
///
/// @sa chaiscript
/// @sa Carbon::CarbonScript
/// @sa Carbon_Language for Built in Functions
/// @sa @ref LangGettingStarted
/// @sa @ref LangKeywordRef
/// @sa @ref LangInPlaceRef
/// @sa @ref LangObjectSystemRef
/// @sa http://www.chaiscript.com
/// @sa http://www.github.com/CarbonScript/CarbonScript
///
/// -----------------------------------------------------------
///
/// @section gettingstarted API Getting Started
///
/// - @ref basics
/// - @ref compiling
/// - @ref eval
/// - @ref adding_items
/// - @ref operatoroverloading
/// - @ref add_class
/// - @ref pointer_conversions
/// - @ref baseclasses
/// - @ref functionobjects
/// - @ref threading
/// - @ref exceptions
///
///
/// @subsection basics Basics
///
/// Basic simple example:
///
/// ~~~~~~~{.cpp}
/// //main.cpp
/// #include <chaiscript/chaiscript.hpp>
///
/// double function(int i, double j)
/// {
///   return i * j;
/// }
///
/// int main()
/// {
///   Carbon::CarbonScript chai;
///   chai.add(Carbon::fun(&function), "function");
///
///   double d = chai.eval<double>("function(3, 4.75);");
/// }
/// ~~~~~~~
///
/// ------------------------------------------------------
///
/// @subsection compiling Compiling CarbonScript Applications
///
/// CarbonScript is a header only library with only one dependency: The
/// operating system provided dynamic library loader, which has to be specified
/// on some platforms.
///
/// @subsubsection compilinggcc Compiling with GCC
///
/// To compile the above application on a Unix like operating system (MacOS,
/// Linux) with GCC you need to link the dynamic loader. For example:
///
/// ~~~~~~~~
/// gcc main.cpp -I/path/to/chaiscript/headers -ldl
/// ~~~~~~~~
///
/// Alternatively, you may compile without threading support.
///
/// ~~~~~~~~
/// gcc main.cpp -I/path/to/chaiscript/headers -ldl -DCARBON_NO_THREADS
/// ~~~~~~~~
///
/// ------------------------------------------
///
/// @subsection eval Evaluating Scripts
///
/// Scripts can be evaluated with the () operator, eval method or eval_file
/// method.
///
/// @subsubsection parenoperator () Operator
///
/// operator() can be used as a handy shortcut for evaluating CarbonScript
/// snippets.
///
/// ~~~~~~~~{.cpp}
/// Carbon::CarbonScript chai;
/// chai("print(@"hello world@")");
/// ~~~~~~~~
///
/// @sa Carbon::CarbonScript::operator()(const std::string &)
///
/// @subsubsection evalmethod Method 'eval'
///
/// The eval method is somewhat more verbose and can be used to get type safely
/// return values from the script.
///
/// ~~~~~~~~{.cpp}
/// Carbon::CarbonScript chai;
/// chai.eval("callsomefunc()");
/// int result = chai.eval<int>("1 + 3");
/// // result now equals 4
/// ~~~~~~~~
///
/// @sa Carbon::CarbonScript::eval
///
/// @subsubsection evalfilemethod Method 'eval_file'
///
/// The 'eval_file' method loads a file from disk and executes the script in it
///
/// ~~~~~~~~{.cpp}
/// Carbon::CarbonScript chai;
/// chai.eval_file("myfile.chai");
/// std::string result = chai.eval_file<std::string>("myfile.chai") // extract
/// the last value returned from the file
/// ~~~~~~~~
///
/// @sa Carbon::CarbonScript::eval_file
///
/// --------------------------------------------------
///
/// @subsection adding_items Adding Items to CarbonScript
///
/// CarbonScript supports 4 basic things that can be added: objects, functions,
/// type infos and Modules
///
/// @subsubsection adding_objects Adding Objects
///
/// Named objects can be created with the Carbon::var function. Note: adding
/// a object adds it to the current thread scope, not to a global scope. If you
/// have multiple threads that need to access the same variables you will need
/// to add them separately for each thread, from the thread itself.
///
/// ~~~~~~~~~{.cpp}
/// using namespace Carbon;
/// CarbonScript chai;
/// int i = 5;
/// chai.add(var(i), "i");
/// chai("print(i)");
/// ~~~~~~~~~
///
/// Immutable objects can be created with the Carbon::const_var function.
///
/// ~~~~~~~~~{.cpp}
/// chai.add(const_var(i), "i");
/// chai("i = 5"); // exception throw, cannot assign const var
/// ~~~~~~~~~
///
/// Named variables can only be accessed from the context they are created in.
/// If you want a global variable, it must be const, and created with the
/// Carbon::CarbonScript::add_global_const function.
///
/// ~~~~~~~~~{.cpp}
/// chai.add_global_const(const_var(i), "i");
/// chai("def somefun() { print(i); }; somefun();");
/// ~~~~~~~~~
///
/// @subsubsection adding_functions Adding Functions
///
/// Functions, methods and members are all added using the same function:
/// Carbon::fun.
///
/// ~~~~~~~~~{.cpp}
/// using namespace Carbon;
///
/// class MyClass {
///   public:
///     int memberdata;
///     void method();
///     void method2(int);
///     static void staticmethod();
///     void overloadedmethod();
///     void overloadedmethod(const std::string &);
/// };
///
/// CarbonScript chai;
/// chai.add(fun(&MyClass::memberdata), "memberdata");
/// chai.add(fun(&MyClass::method), "method");
/// chai.add(fun(&MyClass::staticmethod), "staticmethod");
/// ~~~~~~~~~
///
/// Overloaded methods will need some help, to hint the compiler as to which
/// overload you want:
///
/// ~~~~~~~~~{.cpp}
/// chai.add(fun<void (MyClass::*)()>(&MyClass::overloadedmethod),
/// "overloadedmethod"); chai.add(fun<void (MyClass::*)(const std::string
/// &)>(&MyClass::overloadedmethod), "overloadedmethod");
/// ~~~~~~~~~
///
/// There are also shortcuts built into Carbon::fun for binding up to the
/// first two parameters of the function.
///
/// ~~~~~~~~~{.cpp}
/// MyClass obj;
/// chai.add(fun(&MyClass::method, &obj), "method");
/// chai("method()"); // equiv to obj.method()
/// chai.add(fun(&MyClass::method2, &obj, 3), "method2");
/// chai("method2()"); // equiv to obj.method2(3)
/// ~~~~~~~~~
///
/// @subsubsection addingtypeinfo Adding Type Info
///
/// CarbonScript will automatically support any type implicitly provided to it
/// in the form of objects and function parameters / return types. However, it
/// can be nice to let CarbonScript know more details about the types you are
/// giving it. For instance, the "clone" functionality cannot work unless there
/// is a copy constructor registered and the name of the type is known (so that
/// CarbonScript can look up the copy constructor).
///
/// Continuing with the example "MyClass" from above:
///
/// ~~~~~~~~{.cpp}
/// chai.add(user_type<MyClass>(), "MyClass");
/// ~~~~~~~~
///
/// @subsubsection adding_modules Adding Modules
///
/// Modules are holders for collections of CarbonScript registrations.
///
/// ~~~~~~~~{.cpp}
/// ModulePtr module = get_sum_module();
/// chai.add(module);
/// ~~~~~~~~
///
/// @sa Carbon::Module
///
/// -----------------------------------------------------------------------
///
/// @subsection operatoroverloading Operator Overloading
///
/// Operators are just like any other function in CarbonScript, to overload an
/// operator, simply register it.
///
/// ~~~~~~~~{.cpp}
/// class MyClass {
///   MyClass operator+(const MyClass &) const;
/// };
///
/// chai.add(fun(&MyClass::operator+), "+");
///
/// std::string append_string_int(const std::string &t_lhs, int t_rhs)
/// {
///   std::stringstream ss;
///   ss << t_lhs << t_rhs;
///   return ss.str();
/// }
///
/// chai.add(fun(append_string_int), "+");
/// ~~~~~~~~
///
/// @sa @ref adding_functions
///
/// -----------------------------------------------------------------------
///
/// @subsection add_class Class Helper Utility
///
/// Much of the work of adding new classes to CarbonScript can be reduced with
/// the help of the add_class helper utility.
///
/// ~~~~~~~~{.cpp}
/// class Test
/// {
///   public:
///     void function() {}
///     std::string function2() { return "Function2"; }
///     void function3() {}
///     std::string functionOverload(double) { return "double"; }
///     std::string functionOverload(int) { return "int"; }
/// };
///
/// int main()
/// {
///   Carbon::ModulePtr m = Carbon::ModulePtr(new Carbon::Module());
///
///   Carbon::utility::add_class<Carbon::Test>(*m,
///      "Test",
///      { constructor<Test()>(),
///        constructor<Test(const Test &)>() },
///      { {fun(&Test::function), "function"},
///        {fun(&Test::function2), "function2"},
///        {fun(&Test::function2), "function3"}
///        {fun(static_cast<std::string
///        Test::*(double)>(&Test::functionOverload)), "functionOverload"}
///        {fun(static_cast<std::string Test::*(int)>(&Test::functionOverload)),
///        "functionOverload"} }
///      );
///
///
///   Carbon::CarbonScript chai;
///   chai.add(m);
/// }
/// ~~~~~~~~
///
/// @sa @ref adding_modules
///
/// -----------------------------------------------------------------------
///
/// @subsection pointer_conversions Pointer / Object Conversions
///
/// As much as possible, CarbonScript attempts to convert between &, *, const &,
/// const *, std::shared_ptr<T>, std::shared_ptr<const T>,
/// std::reference_wrapper<T>, std::reference_wrapper<const T> and value types
/// automatically.
///
/// If a Carbon::var object was created in C++ from a pointer, it cannot be
/// converted to a shared_ptr (this would add invalid reference counting). Const
/// may be added, but never removed.
///
/// The take away is that you can pretty much expect function calls to Just Work
/// when you need them to.
///
/// ~~~~~~~~{.cpp}
/// void fun1(const int *);
/// void fun2(int *);
/// void fun3(int);
/// void fun4(int &);
/// void fun5(const int &);
/// void fun5(std::shared_ptr<int>);
/// void fun6(std::shared_ptr<const int>);
/// void fun7(const std::shared_ptr<int> &);
/// void fun8(const std::shared_ptr<const int> &);
/// void fun9(std::reference_wrapper<int>);
/// void fun10(std::reference_wrapper<const int>);
///
/// int main()
/// {
///   using namespace Carbon
///   Carbon::CarbonScript chai;
///   chai.add(fun(fun1), "fun1");
///   chai.add(fun(fun2), "fun2");
///   chai.add(fun(fun3), "fun3");
///   chai.add(fun(fun4), "fun4");
///   chai.add(fun(fun5), "fun5");
///   chai.add(fun(fun6), "fun6");
///   chai.add(fun(fun7), "fun7");
///   chai.add(fun(fun8), "fun8");
///   chai.add(fun(fun9), "fun9");
///   chai.add(fun(fun10), "fun10");
///
///   chai("var i = 10;");
///   chai("fun1(i)");
///   chai("fun2(i)");
///   chai("fun3(i)");
///   chai("fun4(i)");
///   chai("fun5(i)");
///   chai("fun6(i)");
///   chai("fun7(i)");
///   chai("fun8(i)");
///   chai("fun9(i)");
///   chai("fun10(i)");
/// }
/// ~~~~~~~~
///
/// See the unit test unittests/boxed_cast_test.cpp for a complete breakdown of
/// the automatic casts that available and tested.
///
/// -----------------------------------------------------------------------
///
/// @subsection baseclasses Base Classes
///
/// CarbonScript supports handling of passing a derived class object to a
/// function expecting a base class object. For the process to work, the
/// base/derived relationship must be registered with the engine.
///
/// ~~~~~~~~{.cpp}
/// class Base {};
/// class Derived : public Base {};
/// void myfunction(Base *b);
///
/// int main()
/// {
///   Carbon::CarbonScript chai;
///   chai.add(Carbon::base_class<Base, Derived>());
///   Derived d;
///   chai.add(Carbon::var(&d), "d");
///   chai.add(Carbon::fun(&myfunction), "myfunction");
///   chai("myfunction(d)");
/// }
/// ~~~~~~~~
///
/// -----------------------------------------------------------------------
///
///
/// @subsection functionobjects Function Objects
///
/// Functions are first class objects in CarbonScript and CarbonScript supports
/// automatic conversion between CarbonScript functions and std::function
/// objects.
///
/// ~~~~~~~~{.cpp}
/// void callafunc(const std::function<void (const std::string &)> &t_func)
/// {
///   t_func("bob");
/// }
///
/// int main()
/// {
///   Carbon::CarbonScript chai;
///   chai.add(Carbon::fun(&callafunc), "callafunc");
///   chai("callafunc(fun(x) { print(x); })"); // pass a lambda function to the
///   registered function
///                                            // which expects a typed
///                                            std::function
///
///   std::function<void ()> f = chai.eval<std::function<void ()>
///   >("dump_system"); f(); // call the CarbonScript function dump_system, from
///   C++
/// }
/// ~~~~~~~~
///
/// -----------------------------------------------------------------------
///
///
/// @subsection threading Threading
///
/// Thread safety is automatically handled within the CarbonScript system.
/// Objects can be added and scripts executed from multiple threads. For each
/// thread that executes scripts, a new context is created and managed by the
/// engine.
///
/// Thread safety can be disabled by defining CARBON_NO_THREADS when using the
/// library.
///
/// Disabling thread safety increases performance in many cases.
///
/// -----------------------------------------------------------------------
///
///
/// @subsection exceptions Exception Handling
///
/// @subsubsection exceptionsbasics Exception Handling Basics
///
/// Exceptions can be thrown in CarbonScript and caught in C++ or thrown in C++
/// and caught in CarbonScript.
///
/// ~~~~~~~~{.cpp}
/// void throwexception()
/// {
///   throw std::runtime_error("err");
/// }
///
/// int main()
/// {
///   // Throw in C++, catch in CarbonScript
///   Carbon::CarbonScript chai;
///   chai.add(Carbon::fun(&throwexception), "throwexception");
///   chai("try { throwexception(); } catch (e) { print(e.what()); }"); //
///   prints "err"
///
///   // Throw in CarbonScript, catch in C++
///   try {
///     chai("throw(1)");
///   } catch (Carbon::Boxed_Value bv) {
///     int i = Carbon::boxed_cast<int>(bv);
///     // i == 1
///   }
/// }
/// ~~~~~~~~
///
/// @subsubsection exceptionsautomatic Exception Handling Automatic Unboxing
///
/// As an alternative to the manual unboxing of exceptions shown above,
/// exception specifications allow the user to tell CarbonScript what possible
/// exceptions are expected from the script being executed.
///
/// Example:
/// ~~~~~~~~{.cpp}
/// Carbon::CarbonScript chai;
///
/// try {
///   chai.eval("throw(runtime_error(@"error@"))",
///   Carbon::exception_specification<int, double, float, const std::string
///   &, const std::exception &>());
/// } catch (const double e) {
/// } catch (int) {
/// } catch (float) {
/// } catch (const std::string &) {
/// } catch (const std::exception &e) {
///   // This is the one what will be called in the specific throw() above
/// }
/// ~~~~~~~~
///
/// @sa Carbon::Exception_Handler for details on automatic exception
/// unboxing
/// @sa Carbon::exception_specification

/// @page LangObjectSystemRef CarbonScript Language Object Model Reference
///
///
/// CarbonScript has an object system built in, for types defined within the
/// CarbonScript system.
///
/// ~~~~~~~~~
/// attr Rectangle::height
/// attr Rectangle::width
/// def Rectangle::Rectangle() { this.height = 10; this.width = 20 }
/// def Rectangle::area() { this.height * this.width }
///
/// var rect = Rectangle()
/// rect.height = 30
/// print(rect.area())
/// ~~~~~~~~~
///
/// Since CarbonScript 5.4.0 it has been possible to use the "class" keyword to
/// simplify this code.
///
/// ~~~~~~~~~
/// class Rectangle {
///   attr height
///   attr width
///   def Rectangle() { this.height = 10; this.width = 20 }
///   def area() { this.height * this.width }
/// }
///
/// var rect = Rectangle()
/// rect.height = 30
/// print(rect.area())
/// ~~~~~~~~~
///
/// @sa @ref keywordattr
/// @sa @ref keyworddef

/// @page LangInPlaceRef CarbonScript Language In-Place Creation Reference
/// @section inplacevector Vector
///
/// ~~~~~~~~~
/// In-place Vector ::= "[" [expression ("," expression)*]  "]"
/// ~~~~~~~~~
///
/// @section inplacerangedvector Ranged Vector
///
/// ~~~~~~~~~
/// In-place Ranged Vector ::= "[" value ".." value "]"
/// ~~~~~~~~~
///
/// Creates a vector over a range (eg. 1..10)
///
/// @section inplacemap Map
///
/// ~~~~~~~~
/// In-place Map ::= "[" (string ":" expression)+ "]"
/// ~~~~~~~~

/// @page LangGettingStarted CarbonScript Language Getting Started
///
/// CarbonScript is a simple language that should feel familiar to anyone who
/// knows C++ or ECMAScript (JavaScript).
///
/// -----------------------------------------------------------------------
///
/// @section chaiscriptloops Loops
///
/// Common looping constructs exist in CarbonScript
///
/// ~~~~~~~~
/// var i = 0;
/// while (i < 10)
/// {
///   // do something
///   ++i;
/// }
/// ~~~~~~~~
///
/// ~~~~~~~~
/// for (var i = 0; i < 10; ++i)
/// {
///   // do something
/// }
/// ~~~~~~~~
///
/// @sa @ref keywordfor
/// @sa @ref keywordwhile
///
/// -----------------------------------------------------------------------
///
/// @section chaiscriptifs Conditionals
///
/// If statements work as expected
///
/// ~~~~~~~~
/// var b = true;
///
/// if (b) {
///   // do something
/// } else if (c < 10) {
///   // do something else
/// } else {
///   // or do this
/// }
/// ~~~~~~~~
///
/// @sa @ref keywordif
///
/// -----------------------------------------------------------------------
///
/// @section chaiscriptfunctions Functions
///
/// Functions are defined with the def keyword
///
/// ~~~~~~~~
/// def myfun(x) { print(x); }
///
/// myfun(10);
/// ~~~~~~~~
///
/// Functions may have "guards" which determine if which is called.
///
/// ~~~~~~~~
/// eval> def myfun2(x) : x < 10 { print("less than 10"); }
/// eval> def myfun2(x) : x >= 10 { print("10 or greater"); }
/// eval> myfun2(5)
/// less than 10
/// eval> myfun2(12)
/// 10 or greater
/// ~~~~~~~~
///
/// @sa @ref keyworddef
/// @sa @ref keywordattr
/// @sa @ref LangObjectSystemRef
///
/// -----------------------------------------------------------------------
///
/// @section chaiscriptfunctionobjects Function Objects
///
/// Functions are first class types in CarbonScript and can be used as
/// variables.
///
/// ~~~~~~~~
/// eval> var p = print;
/// eval> p(1);
/// 1
/// ~~~~~~~~
///
/// They can also be passed to functions.
///
/// ~~~~~~~~
/// eval> def callfunc(f, lhs, rhs) { return f(lhs, rhs); }
/// eval> def do_something(lhs, rhs) { print("lhs: ${lhs}, rhs: ${rhs}"); }
/// eval> callfunc(do_something, 1, 2);
/// lhs: 1, rhs: 2
/// ~~~~~~~~
///
/// Operators can also be treated as functions by using the back tick operator.
/// Building on the above example:
///
/// ~~~~~~~~
/// eval> callfunc(`+`, 1, 4);
/// 5
/// eval> callfunc(`*`, 3, 2);
/// 6
/// ~~~~~~~~
///
/// -----------------------------------------------------------------------
///
/// @sa @ref LangKeywordRef
/// @sa Carbon_Language for Built in Functions

/// @page LangKeywordRef CarbonScript Language Keyword Reference
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordattr attr
/// Defines a CarbonScript object attribute
///
/// ~~~~~~~~
/// Attribute Definition ::= "attr" class_name "::" attribute_name
/// ~~~~~~~~
///
/// @sa @ref LangObjectSystemRef
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordauto auto
///
/// Defines a variable
///
/// ~~~~~~~~
/// Variable ::= "auto" identifier
/// ~~~~~~~~
///
/// Synonym for @ref keywordvar
///
/// -----------------------------------------------------------------------
///
/// @section keywordbreak break
/// Stops execution of a looping block.
///
/// ~~~~~~~~
/// Break Statement ::= "break"
/// ~~~~~~~~
///
/// @sa @ref keywordfor
/// @sa @ref keywordwhile
///
///
/// -----------------------------------------------------------------------
///
/// @section keyworddef def
/// Begins a function or method definition
///
/// ~~~~~~~~
/// Function Definition ::= "def" identifier "(" [[type] arg ("," [type] arg)*]
/// ")" [":" guard] block Method Definition ::= "def" class_name "::"
/// method_name "(" [[type] arg ("," [type] arg)*] ")" [":" guard] block
/// ~~~~~~~~
///
/// identifier: name of function. Required.
/// args: comma-delimited list of parameter names with optional type specifiers.
/// Optional. guards: guarding statement that act as a prerequisite for the
/// function. Optional. { }: scoped block as function body. Required.
///
/// Functions return values in one of two ways:
///
/// By using an explicit return call, optionally passing the value to be
/// returned. By implicitly returning the value of the last expression (if it is
/// not a while or for loop).
///
/// Method definitions for known types extend those types with new methods. This
/// includes C++ and CarbonScript defined types. Method definitions for unknown
/// types implicitly define the named type.
///
/// @sa @ref LangObjectSystemRef
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordelse else
/// @sa @ref keywordif
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordfor for
/// ~~~~~~~~
/// For Block ::= "for" "(" [initial] ";" stop_condition ";" loop_expression ")"
/// block
/// ~~~~~~~~
/// This loop can be broken using the @ref keywordbreak command.
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordfun fun
/// Begins an anonymous function declaration (sometimes called a lambda).
///
/// ~~~~~~~~
/// Lambda ::= "fun" "(" [variable] ("," variable)*  ")" block
/// ~~~~~~~~
///
/// _Example_
///
/// ~~~~~~~~
/// // Generate an anonymous function object that adds 2 to its parameter
/// var f = fun(x) { x + 2; }
/// ~~~~~~~~
///
/// @sa @ref keyworddef for more details on CarbonScript functions
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordif if
/// Begins a conditional block of code that only executes if the condition
/// evaluates as true.
/// ~~~~~~~~
/// If Block ::= "if" "(" condition ")" block
/// Else If Block ::= "else if" "(" condition ")" block
/// Else Block ::= "else" block
/// ~~~~~~~~
///
/// _Example_
///
/// ~~~~~~~~
/// if (true) {
///   // do something
/// } else if (false) {
///   // do something else
/// } else {
///   // otherwise do this
/// }
/// ~~~~~~~~
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordtry try
/// ~~~~~~~~
/// Try Block ::= "try" block
///  ("catch" ["(" [type] variable ")"] [":" guards] block)+
///    ["finally" block]
/// ~~~~~~~~
///
/// @sa Carbon_Language::throw
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordwhile while
///
/// Begins a conditional block of code that loops 0 or more times, as long as
/// the condition is true
///
/// ~~~~~~~~
/// While Block ::= "while" "(" condition ")" block
/// ~~~~~~~~
///
/// This loop can be broken using the @ref keywordbreak command.
///
///
/// -----------------------------------------------------------------------
///
/// @section keywordvar var
///
/// Defines a variable
///
/// ~~~~~~~~
/// Variable ::= "var" identifier
/// ~~~~~~~~
///
/// Synonym for @ref keywordauto

/// @namespace Carbon
/// @brief Namespace chaiscript contains every API call that the average user
/// will be concerned with.

/// @namespace Carbon::detail
/// @brief Classes and functions reserved for internal use. Items in this
/// namespace are not supported.

#include "basic.hpp"
#include "language/parser.hpp"
#include "stdlib.hpp"

namespace Carbon {
class CarbonScript : public Carbon_Basic {
public:
    CarbonScript(std::vector<std::string> t_modulepaths = {},
                 std::vector<std::string> t_usepaths = {},
                 std::vector<Options> t_opts = Carbon::default_options())
        : Carbon_Basic(Carbon::Std_Lib::library(),
                       std::make_unique<parser::Carbon_Parser<
                           eval::Noop_Tracer, optimizer::Optimizer_Default>>(),
                       std::move(t_modulepaths), std::move(t_usepaths),
                       std::move(t_opts)) {}
};
}  // namespace Carbon

#endif /* CARBON_HPP */
