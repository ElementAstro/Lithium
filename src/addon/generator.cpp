#include "generator.hpp"

#include <sstream>

namespace lithium {
void CppMemberGenerator::generate(const JsonType auto &j, std::ostream &os) {
    for (const auto &member : j) {
        os << "    ";
        if (member.value("is_static", false)) {
            os << "static ";
        }
        if (member.value("is_constexpr", false)) {
            os << "constexpr ";
        }
        os << member["type"].template get<std::string>() << " ";
        os << member["name"].template get<std::string>();

        if (member.contains("bitfield_size")) {
            os << " : " << member["bitfield_size"].template get<int>();
        }

        if (member.contains("default_value")) {
            os << " = " << member["default_value"].template get<std::string>();
        }

        os << ";\n";

        if (member.contains("comment")) {
            os << "    // " << member["comment"].template get<std::string>()
               << "\n";
        }
    }
}

void CppConstructorGenerator::generate(const std::string &className,
                                       const JsonType auto &j,
                                       std::ostream &os) {
    for (const auto &constructor : j) {
        os << "    ";
        if (constructor.value("is_explicit", false)) {
            os << "explicit ";
        }
        os << className << "(";
        bool first = true;
        for (const auto &param : constructor["parameters"]) {
            if (!first)
                os << ", ";
            os << param["type"].template get<std::string>() << " "
               << param["name"].template get<std::string>();
            first = false;
        }
        os << ")";
        generateInitializerList(constructor, os);
        if (constructor.value("is_noexcept", false)) {
            os << " noexcept";
        }
        os << " {\n";
        for (const auto &param : constructor["parameters"]) {
            os << "        this->" << param["name"].template get<std::string>()
               << " = " << param["name"].template get<std::string>() << ";\n";
        }
        os << "    }\n";
    }
    if (j.empty()) {
        os << "    " << className << "() = default;\n";
    }
}

void CppConstructorGenerator::generateInitializerList(
    const JsonType auto &constructor, std::ostream &os) {
    if (constructor.contains("initializer_list") &&
        !constructor["initializer_list"].empty()) {
        os << " : ";
        bool first_init = true;
        for (const auto &init : constructor["initializer_list"]) {
            if (!first_init)
                os << ", ";
            os << init["member"].template get<std::string>() << "("
               << init["value"].template get<std::string>() << ")";
            first_init = false;
        }
    }
}

void CppDestructorGenerator::generate(const JsonType auto &j,
                                      std::ostream &os) {
    if (j.contains("is_virtual") && j["is_virtual"].template get<bool>()) {
        os << "    virtual ~" << j["class_name"].template get<std::string>()
           << "() noexcept = default;\n";
    } else if (j.contains("is_deleted") &&
               j["is_deleted"].template get<bool>()) {
        os << "    ~" << j["class_name"].template get<std::string>()
           << "() = delete;\n";
    } else {
        os << "    ~" << j["class_name"].template get<std::string>()
           << "() noexcept = default;\n";
    }
}

void CppCopyMoveGenerator::generate(const JsonType auto &j, std::ostream &os) {
    const std::string &className = j["class_name"].template get<std::string>();

    if (j.contains("copy_constructor")) {
        if (j["copy_constructor"].template get<bool>()) {
            os << "    " << className << "(const " << className
               << "&) = default;\n";
        } else {
            os << "    " << className << "(const " << className
               << "&) = delete;\n";
        }
    }

    if (j.contains("move_constructor")) {
        if (j["move_constructor"].template get<bool>()) {
            os << "    " << className << "(" << className
               << "&&) noexcept = default;\n";
        } else {
            os << "    " << className << "(" << className
               << "&&) noexcept = delete;\n";
        }
    }

    if (j.contains("copy_assignment")) {
        if (j["copy_assignment"].template get<bool>()) {
            os << "    " << className << "& operator=(const " << className
               << "&) = default;\n";
        } else {
            os << "    " << className << "& operator=(const " << className
               << "&) = delete;\n";
        }
    }

    if (j.contains("move_assignment")) {
        if (j["move_assignment"].template get<bool>()) {
            os << "    " << className << "& operator=(" << className
               << "&&) noexcept = default;\n";
        } else {
            os << "    " << className << "& operator=(" << className
               << "&&) noexcept = delete;\n";
        }
    }
}

void CppMethodGenerator::generate(const JsonType auto &j, std::ostream &os) {
    for (const auto &method : j) {
        os << "    ";
        if (method.value("is_static", false)) {
            os << "static ";
        }
        if (method.value("is_virtual", false)) {
            os << "virtual ";
        }
        if (method.value("is_inline", false)) {
            os << "inline ";
        }
        os << method["return_type"].template get<std::string>() << " "
           << method["name"].template get<std::string>() << "(";

        bool first = true;
        for (const auto &param : method["parameters"]) {
            if (!first)
                os << ", ";
            os << param["type"].template get<std::string>() << " "
               << param["name"].template get<std::string>();
            first = false;
        }

        os << ")";
        if (method.value("is_const", false)) {
            os << " const";
        }
        if (method.value("is_noexcept", false)) {
            os << " noexcept";
        }
        if (method.contains("is_deleted") &&
            method["is_deleted"].template get<bool>()) {
            os << " = delete";
        } else if (method.contains("is_default") &&
                   method["is_default"].template get<bool>()) {
            os << " = default";
        } else {
            os << " {\n";
            os << "        " << method["body"].template get<std::string>()
               << "\n";
            os << "    }";
        }
        os << "\n";
    }
}
void CppAccessorGenerator::generate(const JsonType auto &j, std::ostream &os) {
    for (const auto &accessor : j) {
        os << "    ";
        if (accessor.value("is_static", false)) {
            os << "static ";
        }
        os << accessor["type"].template get<std::string>() << " "
           << accessor["name"].template get<std::string>() << "() const {\n";
        os << "        return "
           << accessor["member"].template get<std::string>() << ";\n";
        os << "    }\n";
    }
}
void CppMutatorGenerator::generate(const JsonType auto &j, std::ostream &os) {
    for (const auto &mutator : j) {
        os << "    void " << mutator["name"].template get<std::string>() << "("
           << mutator["parameter_type"].template get<std::string>()
           << " value) {\n";
        os << "        " << mutator["member"].template get<std::string>()
           << " = value;\n";
        os << "    }\n";
    }
}

void CppFriendFunctionGenerator::generate(const JsonType auto &j,
                                          std::ostream &os) {
    for (const auto &friendFunction : j) {
        os << "    friend "
           << friendFunction["return_type"].template get<std::string>() << " "
           << friendFunction["name"].template get<std::string>() << "(";
        bool first = true;
        for (const auto &param : friendFunction["parameters"]) {
            if (!first) {
                os << ", ";
            }
            os << param["type"].template get<std::string>() << " "
               << param["name"].template get<std::string>();
            first = false;
        }
        os << ")";
        if (friendFunction.value("is_noexcept", false)) {
            os << " noexcept";
        }
        os << ";\n";
    }
}

void CppFriendClassGenerator::generate(const JsonType auto &j,
                                       std::ostream &os) {
    for (const auto &friendClass : j) {
        os << "    friend class " << friendClass.template get<std::string>()
           << ";\n";
    }
}

void CppOperatorOverloadGenerator::generate(const JsonType auto &j,
                                            std::ostream &os) {
    for (const auto &opOverload : j) {
        os << "    ";
        if (opOverload.value("is_static", false)) {
            os << "static ";
        }
        os << opOverload["return_type"].template get<std::string>()
           << " operator" << opOverload["operator"].template get<std::string>()
           << "(";
        bool first = true;
        for (const auto &param : opOverload["parameters"]) {
            if (!first) {
                os << ", ";
            }
            os << param["type"].template get<std::string>() << " "
               << param["name"].template get<std::string>();
            first = false;
        }
        os << ")";
        if (opOverload.value("is_const", false)) {
            os << " const";
        }
        if (opOverload.value("is_noexcept", false)) {
            os << " noexcept";
        }
        os << " {\n";
        os << "        " << opOverload["body"].template get<std::string>()
           << "\n";
        os << "    }\n";
    }
}

void CppCodeGenerator::generate(const std::string &className,
                                const JsonType auto &j, std::ostream &os) {
    generateNamespace(j, os);
    generateNamespaceAlias(j, os);
    generateTemplateParameters(j, os);
    generateEnums(j, os);
    generateClassDeclaration(className, j, os);
    closeNamespace(j, os);
}

void CppCodeGenerator::generateNamespace(const JsonType auto &j,
                                         std::ostream &os) {
    if (j.contains("namespace")) {
        os << "namespace " << j["namespace"].template get<std::string>()
           << " {\n\n";
    }
}

void CppCodeGenerator::closeNamespace(const JsonType auto &j,
                                      std::ostream &os) {
    if (j.contains("namespace")) {
        os << "\n} // namespace " << j["namespace"].template get<std::string>()
           << "\n";
    }
}

void CppCodeGenerator::generateNamespaceAlias(const JsonType auto &j,
                                              std::ostream &os) {
    if (j.contains("namespace_alias")) {
        os << "namespace "
           << j["namespace_alias"]["alias"].template get<std::string>() << " = "
           << j["namespace_alias"]["namespace"].template get<std::string>()
           << ";\n\n";
    }
}

void CppCodeGenerator::generateTemplateParameters(const JsonType auto &j,
                                                  std::ostream &os) {
    if (j.contains("template_parameters")) {
        os << "template <";

        bool first = true;
        for (const auto &param : j["template_parameters"]) {
            if (!first) {
                os << ", ";
            }
            os << "typename " << param.template get<std::string>();
            first = false;
        }

        os << ">\n";
    }
}

void CppCodeGenerator::generateEnums(const JsonType auto &j, std::ostream &os) {
    if (j.contains("enums")) {
        for (const auto &enumDef : j["enums"]) {
            if (enumDef.value("is_class_enum", false)) {
                os << "enum class "
                   << enumDef["name"].template get<std::string>() << " {\n";
            } else {
                os << "enum " << enumDef["name"].template get<std::string>()
                   << " {\n";
            }
            for (const auto &enumValue : enumDef["values"]) {
                os << "    " << enumValue.template get<std::string>() << ",\n";
            }
            os << "};\n\n";
        }
    }
}

void generateClassDeclaration(const std::string &className,
                              const JsonType auto &j, std::ostream &os) {
    os << "class " << className;

    generateBaseClasses(j, os);

    os << " {\n";
    generateAccessModifiers(j, os, "public");
    CppMemberGenerator::generate(j["members"], os);
    generateAccessModifiers(j, os, "protected");
    generateAccessModifiers(j, os, "private");
    CppConstructorGenerator::generate(className, j["constructors"], os);
    CppDestructorGenerator::generate(j["destructor"], os);
    CppCopyMoveGenerator::generate(j, os);
    CppMethodGenerator::generate(j["methods"], os);
    if (j.contains("accessors")) {
        CppAccessorGenerator::generate(j["accessors"], os);
    }
    if (j.contains("mutators")) {
        CppMutatorGenerator::generate(j["mutators"], os);
    }
    if (j.contains("friend_functions")) {
        CppFriendFunctionGenerator::generate(j["friend_functions"], os);
    }
    if (j.contains("friend_classes")) {
        CppFriendClassGenerator::generate(j["friend_classes"], os);
    }
    if (j.contains("operator_overloads")) {
        CppOperatorOverloadGenerator::generate(j["operator_overloads"], os);
    }

    os << "};\n";
}

void CppCodeGenerator::generateBaseClasses(const JsonType auto &j,
                                           std::ostream &os) {
    if (j.contains("base_classes")) {
        os << " : ";

        bool first = true;
        for (const auto &baseClass : j["base_classes"]) {
            if (!first)
                os << ", ";
            os << "public " << baseClass.template get<std::string>();
            first = false;
        }
    }
}

void CppCodeGenerator::generateAccessModifiers(const JsonType auto &j,
                                               std::ostream &os,
                                               const std::string &modifier) {
    if (j.contains(modifier)) {
        os << modifier << ":\n";
        if (j[modifier].contains("members")) {
            CppMemberGenerator::generate(j[modifier]["members"], os);
        }
        if (j[modifier].contains("methods")) {
            CppMethodGenerator::generate(j[modifier]["methods"], os);
        }
    }
}
}  // namespace lithium
