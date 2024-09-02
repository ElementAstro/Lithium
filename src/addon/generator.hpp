#ifndef LITHIUM_ADDON_GENERATOR_HPP
#define LITHIUM_ADDON_GENERATOR_HPP

#include <concepts>
#include <string>

namespace lithium {
template <typename T>
concept JsonType = requires(T t, const std::string &key) {
    { t.contains(key) } -> std::convertible_to<bool>;
    { t[key] } -> std::convertible_to<std::string>;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::input_iterator;
};

/**
 * @brief Generates C++ member declarations.
 */
class CppMemberGenerator {
public:
    /**
     * @brief Generates C++ member declarations based on JSON input.
     *
     * @param j JSON object containing member information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ constructors.
 */
class CppConstructorGenerator {
public:
    /**
     * @brief Generates C++ constructor definitions based on JSON input.
     *
     * @param className The name of the class for which constructors are being
     * generated.
     * @param j JSON object containing constructor information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const std::string &className, const JsonType auto &j,
                         std::ostream &os);

private:
    /**
     * @brief Generates the initializer list for a constructor.
     *
     * @param constructor JSON object containing constructor information.
     * @param os Output stream to which the initializer list will be written.
     */
    static void generateInitializerList(const JsonType auto &constructor,
                                        std::ostream &os);
};

/**
 * @brief Generates C++ destructors.
 */
class CppDestructorGenerator {
public:
    /**
     * @brief Generates C++ destructor definitions based on JSON input.
     *
     * @param j JSON object containing destructor information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ copy and move constructors and assignment operators.
 */
class CppCopyMoveGenerator {
public:
    /**
     * @brief Generates C++ copy and move constructors and assignment operators
     * based on JSON input.
     *
     * @param j JSON object containing copy and move operations information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ methods.
 */
class CppMethodGenerator {
public:
    /**
     * @brief Generates C++ method definitions based on JSON input.
     *
     * @param j JSON object containing method information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ accessors (getter methods).
 */
class CppAccessorGenerator {
public:
    /**
     * @brief Generates C++ accessor methods based on JSON input.
     *
     * @param j JSON object containing accessor information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ mutators (setter methods).
 */
class CppMutatorGenerator {
public:
    /**
     * @brief Generates C++ mutator methods based on JSON input.
     *
     * @param j JSON object containing mutator information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ friend functions.
 */
class CppFriendFunctionGenerator {
public:
    /**
     * @brief Generates C++ friend function declarations and definitions based
     * on JSON input.
     *
     * @param j JSON object containing friend function information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ friend class declarations.
 */
class CppFriendClassGenerator {
public:
    /**
     * @brief Generates C++ friend class declarations based on JSON input.
     *
     * @param j JSON object containing friend class information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates C++ operator overloads.
 */
class CppOperatorOverloadGenerator {
public:
    /**
     * @brief Generates C++ operator overload definitions based on JSON input.
     *
     * @param j JSON object containing operator overload information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const JsonType auto &j, std::ostream &os);
};

/**
 * @brief Generates complete C++ class code including declarations and
 * definitions.
 */
class CppCodeGenerator {
public:
    /**
     * @brief Generates the complete C++ class code based on JSON input.
     *
     * @param className The name of the class to be generated.
     * @param j JSON object containing class information.
     * @param os Output stream to which the generated code will be written.
     */
    static void generate(const std::string &className, const JsonType auto &j,
                         std::ostream &os);

private:
    /**
     * @brief Generates the namespace block for the class.
     *
     * @param j JSON object containing namespace information.
     * @param os Output stream to which the namespace block will be written.
     */
    static void generateNamespace(const JsonType auto &j, std::ostream &os);

    /**
     * @brief Closes the namespace block for the class.
     *
     * @param j JSON object containing namespace information.
     * @param os Output stream to which the closing namespace block will be
     * written.
     */
    static void closeNamespace(const JsonType auto &j, std::ostream &os);

    /**
     * @brief Generates the namespace alias declarations based on JSON input.
     *
     * @param j JSON object containing namespace alias information.
     * @param os Output stream to which the namespace alias declarations will be
     * written.
     */
    static void generateNamespaceAlias(const JsonType auto &j,
                                       std::ostream &os);

    /**
     * @brief Generates template parameters for the class based on JSON input.
     *
     * @param j JSON object containing template parameter information.
     * @param os Output stream to which the template parameters will be written.
     */
    static void generateTemplateParameters(const JsonType auto &j,
                                           std::ostream &os);

    /**
     * @brief Generates enum declarations based on JSON input.
     *
     * @param j JSON object containing enum information.
     * @param os Output stream to which the enum declarations will be written.
     */
    static void generateEnums(const JsonType auto &j, std::ostream &os);

    /**
     * @brief Generates the class declaration including its members.
     *
     * @param className The name of the class to be declared.
     * @param j JSON object containing class declaration information.
     * @param os Output stream to which the class declaration will be written.
     */
    static void generateClassDeclaration(const std::string &className,
                                         const JsonType auto &j,
                                         std::ostream &os);

    /**
     * @brief Generates base class declarations based on JSON input.
     *
     * @param j JSON object containing base class information.
     * @param os Output stream to which the base class declarations will be
     * written.
     */
    static void generateBaseClasses(const JsonType auto &j, std::ostream &os);

    /**
     * @brief Generates access modifiers (public, protected, private) for the
     * class based on JSON input.
     *
     * @param j JSON object containing access modifier information.
     * @param os Output stream to which the access modifiers will be written.
     * @param modifier The access modifier to be applied (public, protected,
     * private).
     */
    static void generateAccessModifiers(const JsonType auto &j,
                                        std::ostream &os,
                                        const std::string &modifier);
};

}  // namespace lithium

#endif
