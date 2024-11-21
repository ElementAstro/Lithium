#include "manager.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <dlfcn.h>
#include <ffi.h>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/macro.hpp"
#include "atom/utils/print.hpp"

namespace fs = std::filesystem;

namespace lithium {
// Token种类
enum class TokenType {
    NUMBER,             // 数字（整数和浮点数）
    STRING,             // 字符串字面量
    PLUS,               // +
    MINUS,              // -
    MULTIPLY,           // *
    DIVIDE,             // /
    IDENTIFIER,         // 标识符
    ASSIGNMENT,         // =
    SEMICOLON,          // ;
    COLON,              // :
    DOUBLE_COLON,       // ::
    LEFT_PARENTHESIS,   // (
    RIGHT_PARENTHESIS,  // )
    LEFT_BRACKET,       // [
    RIGHT_BRACKET,      // ]
    LEFT_BRACE,         // {
    RIGHT_BRACE,        // }
    IF,                 // if
    ELSE,               // else
    WHILE,              // while
    FOR,                // for
    SWITCH,             // switch
    CASE,               // case
    FUNCTION,           // function
    RETURN,             // return
    IMPORT,             // import
    AS,                 // as
    TRY,                // try
    CATCH,              // catch
    THROW,              // throw
    GOTO,               // goto
    LABEL,              // label
    CLASS,              // class
    ENUM_CLASS,         // enum class
    COMMA,              // ,
    GREATER,            // >
    LESS,               // <
    GREATER_EQUAL,      // >=
    LESS_EQUAL,         // <=
    EQUAL,              // ==
    NOT_EQUAL,          // !=
    AND,                // &&
    OR,                 // ||
    NOT,                // !
    END_OF_FILE,        // 文件结束
};

// Token结构
struct Token {
    TokenType type;
    std::string value;
} ATOM_ALIGNAS(64);

// AST节点基类
struct ASTNode {
    virtual ~ASTNode() = default;
    [[nodiscard]] virtual auto clone() const -> std::shared_ptr<ASTNode> = 0;
};

// 数字节点
struct Number : ASTNode {
private:
    std::string value_;

public:
    explicit Number(std::string val) : value_(std::move(val)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<Number>(*this);
    }

    [[nodiscard]] auto getValue() const -> const std::string& { return value_; }
} ATOM_ALIGNAS(32);

// 字符串节点
struct StringLiteral : ASTNode {
private:
    std::string value_;

public:
    explicit StringLiteral(std::string val) : value_(std::move(val)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<StringLiteral>(*this);
    }

    [[nodiscard]] auto getValue() const -> const std::string& { return value_; }
} ATOM_ALIGNAS(32);

// 标识符节点
struct Identifier : ASTNode {
private:
    std::string name_;

public:
    explicit Identifier(std::string n) : name_(std::move(n)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<Identifier>(*this);
    }

    [[nodiscard]] auto getName() const -> const std::string& { return name_; }
} ATOM_ALIGNAS(32);

// 二元操作节点
struct BinaryOp : ASTNode {
private:
    std::shared_ptr<ASTNode> left_;
    std::shared_ptr<ASTNode> right_;
    Token opToken_;

public:
    BinaryOp(std::shared_ptr<ASTNode> lhs, Token opr,
             std::shared_ptr<ASTNode> rhs)
        : left_(std::move(lhs)),
          right_(std::move(rhs)),
          opToken_(std::move(opr)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<BinaryOp>(left_->clone(), opToken_,
                                          right_->clone());
    }

    [[nodiscard]] auto getLeft() const -> const std::shared_ptr<ASTNode>& {
        return left_;
    }

    [[nodiscard]] auto getRight() const -> const std::shared_ptr<ASTNode>& {
        return right_;
    }

    [[nodiscard]] auto getOp() const -> const Token& { return opToken_; }
} ATOM_ALIGNAS(128);

// 赋值语句节点
struct Assignment : ASTNode {
private:
    std::shared_ptr<Identifier> identifier_;
    std::shared_ptr<ASTNode> value_;

public:
    Assignment(std::shared_ptr<Identifier> ident, std::shared_ptr<ASTNode> val)
        : identifier_(std::move(ident)), value_(std::move(val)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<Assignment>(
            std::dynamic_pointer_cast<Identifier>(identifier_->clone()),
            value_->clone());
    }

    [[nodiscard]] auto getIdentifier() const
        -> const std::shared_ptr<Identifier>& {
        return identifier_;
    }

    [[nodiscard]] auto getValue() const -> const std::shared_ptr<ASTNode>& {
        return value_;
    }
} ATOM_ALIGNAS(32);

// 表达式语句节点
struct ExpressionStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> expression_;

public:
    explicit ExpressionStatement(std::shared_ptr<ASTNode> expr)
        : expression_(std::move(expr)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<ExpressionStatement>(expression_->clone());
    }

    [[nodiscard]] auto getExpression() const
        -> const std::shared_ptr<ASTNode>& {
        return expression_;
    }
} ATOM_ALIGNAS(16);

// 块语句节点
struct BlockStatement : ASTNode {
private:
    std::vector<std::shared_ptr<ASTNode>> statements_;

public:
    BlockStatement() = default;

    void addStatement(std::shared_ptr<ASTNode> stmt) {
        statements_.push_back(std::move(stmt));
    }

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        auto newBlock = std::make_shared<BlockStatement>();
        for (const auto& stmt : statements_) {
            newBlock->addStatement(stmt->clone());
        }
        return newBlock;
    }

    [[nodiscard]] auto getStatements() const
        -> const std::vector<std::shared_ptr<ASTNode>>& {
        return statements_;
    }
} ATOM_ALIGNAS(32);

// if语句节点
struct IfStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> condition_;
    std::shared_ptr<ASTNode> thenBranch_;
    std::shared_ptr<ASTNode> elseBranch_;

public:
    IfStatement(std::shared_ptr<ASTNode> cond, std::shared_ptr<ASTNode> thenBr,
                std::shared_ptr<ASTNode> elseBr = nullptr)
        : condition_(std::move(cond)),
          thenBranch_(std::move(thenBr)),
          elseBranch_(std::move(elseBr)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<IfStatement>(
            condition_->clone(), thenBranch_->clone(),
            elseBranch_ ? elseBranch_->clone() : nullptr);
    }

    [[nodiscard]] auto getCondition() const -> const std::shared_ptr<ASTNode>& {
        return condition_;
    }

    [[nodiscard]] auto getThenBranch() const
        -> const std::shared_ptr<ASTNode>& {
        return thenBranch_;
    }

    [[nodiscard]] auto getElseBranch() const
        -> const std::shared_ptr<ASTNode>& {
        return elseBranch_;
    }
} ATOM_ALIGNAS(64);

// while语句节点
struct WhileStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> condition_;
    std::shared_ptr<ASTNode> body_;

public:
    WhileStatement(std::shared_ptr<ASTNode> cond,
                   std::shared_ptr<ASTNode> bodyStmt)
        : condition_(std::move(cond)), body_(std::move(bodyStmt)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<WhileStatement>(condition_->clone(),
                                                body_->clone());
    }

    [[nodiscard]] auto getCondition() const -> const std::shared_ptr<ASTNode>& {
        return condition_;
    }

    [[nodiscard]] auto getBody() const -> const std::shared_ptr<ASTNode>& {
        return body_;
    }
} ATOM_ALIGNAS(32);

// for语句节点
struct ForStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> initializer_;
    std::shared_ptr<ASTNode> condition_;
    std::shared_ptr<ASTNode> increment_;
    std::shared_ptr<ASTNode> body_;

public:
    ForStatement(std::shared_ptr<ASTNode> init, std::shared_ptr<ASTNode> cond,
                 std::shared_ptr<ASTNode> incr, std::shared_ptr<ASTNode> body)
        : initializer_(std::move(init)),
          condition_(std::move(cond)),
          increment_(std::move(incr)),
          body_(std::move(body)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<ForStatement>(
            initializer_->clone(), condition_->clone(), increment_->clone(),
            body_->clone());
    }

    [[nodiscard]] auto getInitializer() const
        -> const std::shared_ptr<ASTNode>& {
        return initializer_;
    }
    [[nodiscard]] auto getCondition() const -> const std::shared_ptr<ASTNode>& {
        return condition_;
    }
    [[nodiscard]] auto getIncrement() const -> const std::shared_ptr<ASTNode>& {
        return increment_;
    }
    [[nodiscard]] auto getBody() const -> const std::shared_ptr<ASTNode>& {
        return body_;
    }
} ATOM_ALIGNAS(64);

// switch-case语句节点
struct SwitchStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> condition_;
    std::vector<
        std::pair<std::shared_ptr<ASTNode>, std::shared_ptr<BlockStatement>>>
        cases_;

public:
    SwitchStatement(std::shared_ptr<ASTNode> cond,
                    std::vector<std::pair<std::shared_ptr<ASTNode>,
                                          std::shared_ptr<BlockStatement>>>
                        cases)
        : condition_(std::move(cond)), cases_(std::move(cases)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        std::vector<std::pair<std::shared_ptr<ASTNode>,
                              std::shared_ptr<BlockStatement>>>
            clonedCases;
        clonedCases.reserve(cases_.size());
        for (const auto& [caseValue, caseBlock] : cases_) {
            clonedCases.emplace_back(
                caseValue->clone(),
                std::dynamic_pointer_cast<BlockStatement>(caseBlock->clone()));
        }
        return std::make_shared<SwitchStatement>(condition_->clone(),
                                                 std::move(clonedCases));
    }

    [[nodiscard]] auto getCondition() const -> const std::shared_ptr<ASTNode>& {
        return condition_;
    }

    [[nodiscard]] auto getCases() const
        -> const std::vector<std::pair<std::shared_ptr<ASTNode>,
                                       std::shared_ptr<BlockStatement>>>& {
        return cases_;
    }
} ATOM_ALIGNAS(64);

// 函数定义节点
struct FunctionDef : ASTNode {
private:
    std::string name_;
    std::vector<std::string> params_;
    std::shared_ptr<BlockStatement> body_;

public:
    FunctionDef(std::string funcName, std::vector<std::string> paramList,
                std::shared_ptr<BlockStatement> bodyStmt)
        : name_(std::move(funcName)),
          params_(std::move(paramList)),
          body_(std::move(bodyStmt)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<FunctionDef>(
            name_, params_,
            std::dynamic_pointer_cast<BlockStatement>(body_->clone()));
    }

    [[nodiscard]] auto getName() const -> const std::string& { return name_; }

    [[nodiscard]] auto getParams() const -> const std::vector<std::string>& {
        return params_;
    }

    [[nodiscard]] auto getBody() const
        -> const std::shared_ptr<BlockStatement>& {
        return body_;
    }
} ATOM_ALIGNAS(128);

struct ClassDefinition : ASTNode {
private:
    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> members_;

public:
    ClassDefinition(
        std::string className,
        std::unordered_map<std::string, std::shared_ptr<ASTNode>> members)
        : name_(std::move(className)), members_(std::move(members)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        std::unordered_map<std::string, std::shared_ptr<ASTNode>> clonedMembers;
        for (const auto& [name, member] : members_) {
            clonedMembers[name] = member->clone();
        }
        return std::make_shared<ClassDefinition>(name_,
                                                 std::move(clonedMembers));
    }

    auto getName() const -> const std::string& { return name_; }
    auto getMembers() const
        -> const std::unordered_map<std::string, std::shared_ptr<ASTNode>>& {
        return members_;
    }
} ATOM_ALIGNAS(128);

// 枚举类定义节点
struct EnumClassDefinition : ASTNode {
private:
    std::string name_;
    std::vector<std::string> enumerators_;

public:
    EnumClassDefinition(std::string enumName,
                        std::vector<std::string> enumerators)
        : name_(std::move(enumName)), enumerators_(std::move(enumerators)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<EnumClassDefinition>(name_, enumerators_);
    }

    [[nodiscard]] auto getName() const -> const std::string& { return name_; }
    [[nodiscard]] auto getEnumerators() const
        -> const std::vector<std::string>& {
        return enumerators_;
    }
} ATOM_ALIGNAS(64);

// 返回语句节点
struct ReturnStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> value_;

public:
    explicit ReturnStatement(std::shared_ptr<ASTNode> val)
        : value_(std::move(val)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<ReturnStatement>(value_->clone());
    }

    [[nodiscard]] auto getValue() const -> const std::shared_ptr<ASTNode>& {
        return value_;
    }
} ATOM_ALIGNAS(16);

// Import语句节点
struct ImportStatement : ASTNode {
private:
    std::string moduleName_;
    std::string alias_;

public:
    ImportStatement(std::string modName, std::string aliasName)
        : moduleName_(std::move(modName)), alias_(std::move(aliasName)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<ImportStatement>(moduleName_, alias_);
    }

    [[nodiscard]] auto getModuleName() const -> const std::string& {
        return moduleName_;
    }

    [[nodiscard]] auto getAlias() const -> const std::string& { return alias_; }
} ATOM_ALIGNAS(64);

// try-catch语句节点
struct TryCatchStatement : ASTNode {
private:
    std::shared_ptr<BlockStatement> tryBlock_;
    std::shared_ptr<BlockStatement> catchBlock_;

public:
    TryCatchStatement(std::shared_ptr<BlockStatement> tryBlk,
                      std::shared_ptr<BlockStatement> catchBlk)
        : tryBlock_(std::move(tryBlk)), catchBlock_(std::move(catchBlk)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<TryCatchStatement>(
            std::dynamic_pointer_cast<BlockStatement>(tryBlock_->clone()),
            std::dynamic_pointer_cast<BlockStatement>(catchBlock_->clone()));
    }

    [[nodiscard]] auto getTryBlock() const
        -> const std::shared_ptr<BlockStatement>& {
        return tryBlock_;
    }

    [[nodiscard]] auto getCatchBlock() const
        -> const std::shared_ptr<BlockStatement>& {
        return catchBlock_;
    }
} ATOM_ALIGNAS(32);

// throw语句节点
struct ThrowStatement : ASTNode {
private:
    std::shared_ptr<ASTNode> expression_;

public:
    explicit ThrowStatement(std::shared_ptr<ASTNode> expr)
        : expression_(std::move(expr)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<ThrowStatement>(expression_->clone());
    }

    [[nodiscard]] auto getExpression() const
        -> const std::shared_ptr<ASTNode>& {
        return expression_;
    }
} ATOM_ALIGNAS(16);

// goto语句节点
struct GotoStatement : ASTNode {
private:
    std::string label_;

public:
    explicit GotoStatement(std::string label) : label_(std::move(label)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<GotoStatement>(label_);
    }

    [[nodiscard]] auto getLabel() const -> const std::string& { return label_; }
} ATOM_ALIGNAS(32);

// 标签语句节点
struct LabelStatement : ASTNode {
private:
    std::string label_;

public:
    explicit LabelStatement(std::string label) : label_(std::move(label)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        return std::make_shared<LabelStatement>(label_);
    }

    [[nodiscard]] auto getLabel() const -> const std::string& { return label_; }
} ATOM_ALIGNAS(32);

// 函数调用节点
struct FunctionCall : ASTNode {
private:
    std::string name_;
    std::vector<std::shared_ptr<ASTNode>> arguments_;

public:
    FunctionCall(std::string funcName,
                 std::vector<std::shared_ptr<ASTNode>> args)
        : name_(std::move(funcName)), arguments_(std::move(args)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        std::vector<std::shared_ptr<ASTNode>> clonedArgs;
        clonedArgs.reserve(arguments_.size());
        for (const auto& arg : arguments_) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_shared<FunctionCall>(name_, std::move(clonedArgs));
    }

    [[nodiscard]] auto getName() const -> const std::string& { return name_; }

    [[nodiscard]] auto getArguments() const
        -> const std::vector<std::shared_ptr<ASTNode>>& {
        return arguments_;
    }
} ATOM_ALIGNAS(64);

// 程序节点
struct Program : ASTNode {
private:
    std::vector<std::shared_ptr<ASTNode>> statements_;

public:
    Program() = default;

    explicit Program(std::vector<std::shared_ptr<ASTNode>> stmts)
        : statements_(std::move(stmts)) {}

    void addStatement(std::shared_ptr<ASTNode> stmt) {
        statements_.push_back(std::move(stmt));
    }

    [[nodiscard]] auto clone() const -> std::shared_ptr<ASTNode> override {
        auto newProgram = std::make_shared<Program>();
        for (const auto& stmt : statements_) {
            newProgram->addStatement(stmt->clone());
        }
        return newProgram;
    }

    [[nodiscard]] auto getStatements() const
        -> const std::vector<std::shared_ptr<ASTNode>>& {
        return statements_;
    }
} ATOM_ALIGNAS(32);

// Lexer类
class Lexer {
public:
    explicit Lexer(std::string source) : src_(std::move(source)) {}

    auto nextToken() -> Token {
        while (current_ < src_.size()) {
            char ch = src_[current_];

            if (std::isspace(ch) != 0) {
                ++current_;
                continue;
            }

            if (ch == '"') {
                return stringLiteral();
            }

            if (ch == '[') {
                ++current_;
                return {TokenType::LEFT_BRACKET, "["};
            }

            if (ch == ']') {
                ++current_;
                return {TokenType::RIGHT_BRACKET, "]"};
            }

            if (std::isdigit(ch) || (ch == '.' && current_ + 1 < src_.size() &&
                                     (std::isdigit(src_[current_ + 1]) != 0))) {
                return number();
            }

            if ((std::isalpha(ch) != 0) || ch == '_') {
                return identifier();
            }

            // 处理单行注释
            if (ch == '/' && peekNext() == '/') {
                while (current_ < src_.size() && src_[current_] != '\n') {
                    ++current_;
                }
                continue;
            }

            // 处理多行注释
            if (ch == '/' && peekNext() == '*') {
                current_ += 2;  // 跳过 /*
                while (current_ < src_.size() &&
                       !(src_[current_] == '*' && peekNext() == '/')) {
                    ++current_;
                }
                if (current_ < src_.size()) {
                    current_ += 2;  // 跳过 */
                }
                continue;
            }

            // 处理多字符运算符
            if (ch == '>') {
                if (peekNext() == '=') {
                    current_ += 2;
                    return {TokenType::GREATER_EQUAL, ">="};
                }
                ++current_;
                return {TokenType::GREATER, ">"};
            }

            if (ch == '<') {
                if (peekNext() == '=') {
                    current_ += 2;
                    return {TokenType::LESS_EQUAL, "<="};
                }
                ++current_;
                return {TokenType::LESS, "<"};
            }

            if (ch == '=') {
                if (peekNext() == '=') {
                    current_ += 2;
                    return {TokenType::EQUAL, "=="};
                }
                ++current_;
                return {TokenType::ASSIGNMENT, "="};
            }

            if (ch == '!') {
                if (peekNext() == '=') {
                    current_ += 2;
                    return {TokenType::NOT_EQUAL, "!="};
                }
                ++current_;
                return {TokenType::NOT, "!"};
            }

            if (ch == ':' && peekNext() == ':') {
                current_ += 2;
                return {TokenType::DOUBLE_COLON, "::"};
            }

            if (ch == ':') {
                ++current_;
                return {TokenType::COLON, ":"};
            }

            if (ch == '&' && peekNext() == '&') {
                current_ += 2;
                return {TokenType::AND, "&&"};
            }

            if (ch == '|' && peekNext() == '|') {
                current_ += 2;
                return {TokenType::OR, "||"};
            }

            // 单字符Token
            switch (ch) {
                case '+':
                    ++current_;
                    return {TokenType::PLUS, "+"};
                case '-':
                    ++current_;
                    return {TokenType::MINUS, "-"};
                case '*':
                    ++current_;
                    return {TokenType::MULTIPLY, "*"};
                case '/':
                    ++current_;
                    return {TokenType::DIVIDE, "/"};
                case ';':
                    ++current_;
                    return {TokenType::SEMICOLON, ";"};
                case '(':
                    ++current_;
                    return {TokenType::LEFT_PARENTHESIS, "("};
                case ')':
                    ++current_;
                    return {TokenType::RIGHT_PARENTHESIS, ")"};
                case '{':
                    ++current_;
                    return {TokenType::LEFT_BRACE, "{"};
                case '}':
                    ++current_;
                    return {TokenType::RIGHT_BRACE, "}"};
                case ',':
                    ++current_;
                    return {TokenType::COMMA, ","};
                default:
                    THROW_RUNTIME_ERROR(std::string("Unexpected character: ") +
                                        ch);
            }
        }

        return {TokenType::END_OF_FILE, ""};
    }

private:
    std::string src_;
    size_t current_ = 0;

    auto peekNext() const -> char {
        if (current_ + 1 < src_.size()) {
            return src_[current_ + 1];
        }
        return '\0';
    }

    [[nodiscard]] auto number() -> Token {
        size_t start = current_;
        bool hasDot = false;
        while (current_ < src_.size() &&
               ((std::isdigit(src_[current_]) != 0) || src_[current_] == '.')) {
            if (src_[current_] == '.') {
                if (hasDot) {
                    THROW_RUNTIME_ERROR(
                        "Invalid number format with multiple dots");
                }
                hasDot = true;
            }
            ++current_;
        }
        return {TokenType::NUMBER, src_.substr(start, current_ - start)};
    }

    [[nodiscard]] auto stringLiteral() -> Token {
        ++current_;  // 跳过起始的双引号
        size_t start = current_;
        while (current_ < src_.size() && src_[current_] != '"') {
            if (src_[current_] == '\\' && current_ + 1 < src_.size()) {
                current_ += 2;  // 跳过转义字符
            } else {
                ++current_;
            }
        }
        if (current_ >= src_.size()) {
            THROW_RUNTIME_ERROR("Unterminated string literal");
        }
        std::string str = src_.substr(start, current_ - start);
        ++current_;  // 跳过结束的双引号
        return {TokenType::STRING, str};
    }

    [[nodiscard]] auto identifier() -> Token {
        size_t start = current_;
        while (current_ < src_.size() &&
               ((std::isalnum(src_[current_]) != 0) || src_[current_] == '_')) {
            ++current_;
        }
        std::string word = src_.substr(start, current_ - start);

        if (word == "if") {
            return {TokenType::IF, word};
        }
        if (word == "else") {
            return {TokenType::ELSE, word};
        }
        if (word == "while") {
            return {TokenType::WHILE, word};
        }
        if (word == "switch") {
            return {TokenType::SWITCH, word};
        }
        if (word == "case") {
            return {TokenType::CASE, word};
        }
        if (word == "function") {
            return {TokenType::FUNCTION, word};
        }
        if (word == "return") {
            return {TokenType::RETURN, word};
        }
        if (word == "import") {
            return {TokenType::IMPORT, word};
        }
        if (word == "as") {
            return {TokenType::AS, word};
        }
        if (word == "try") {
            return {TokenType::TRY, word};
        }
        if (word == "catch") {
            return {TokenType::CATCH, word};
        }
        if (word == "throw") {
            return {TokenType::THROW, word};
        }
        if (word == "goto") {
            return {TokenType::GOTO, word};
        }
        if (word == "class") {
            return {TokenType::CLASS, word};
        }
        if (word == "enum") {
            if (peekNext() == ' ' && src_.substr(current_ + 1, 5) == "class") {
                current_ += 6;  // 跳过 "enum class"
                return {TokenType::ENUM_CLASS, "enum class"};
            }
        }
        if (word == "for") {
            return {TokenType::FOR, word};
        }

        return {TokenType::IDENTIFIER, word};
    }
};

// Parser类
class Parser {
public:
    explicit Parser(Lexer& lex)
        : lexer_(lex), currentToken_(lexer_.nextToken()) {}

    [[nodiscard]] auto parse() -> std::shared_ptr<Program> {
        auto program = std::make_shared<Program>();
        while (currentToken_.type != TokenType::END_OF_FILE) {
            program->addStatement(statement());
        }
        return program;
    }

private:
    Lexer& lexer_;
    Token currentToken_;

    void consume(TokenType type) {
        if (currentToken_.type == type) {
            currentToken_ = lexer_.nextToken();
        } else {
            THROW_RUNTIME_ERROR("Unexpected token: " + currentToken_.value);
        }
    }

    [[nodiscard]] auto statement() -> std::shared_ptr<ASTNode> {
        if (currentToken_.type == TokenType::IDENTIFIER) {
            // 可能是赋值或函数调用
            std::string identifierName = currentToken_.value;
            consume(TokenType::IDENTIFIER);
            if (currentToken_.type == TokenType::ASSIGNMENT) {
                // 赋值语句
                auto idNode = std::make_shared<Identifier>(identifierName);
                consume(TokenType::ASSIGNMENT);
                auto value = expression();
                consume(TokenType::SEMICOLON);
                return std::make_shared<Assignment>(std::move(idNode),
                                                    std::move(value));
            }
            if (currentToken_.type == TokenType::LEFT_PARENTHESIS) {
                // 函数调用语句
                auto funcCall = parseFunctionCall(identifierName);
                consume(TokenType::SEMICOLON);
                return std::make_shared<ExpressionStatement>(
                    std::move(funcCall));
            }
            if (currentToken_.type == TokenType::COLON) {
                // 标签语句
                consume(TokenType::COLON);
                return std::make_shared<LabelStatement>(identifierName);
            }
            THROW_RUNTIME_ERROR("Unexpected token after identifier: " +
                                currentToken_.value);
        }
        if (currentToken_.type == TokenType::IF) {
            return ifStatement();
        }
        if (currentToken_.type == TokenType::WHILE) {
            return whileStatement();
        }
        if (currentToken_.type == TokenType::FOR) {
            return forStatement();
        }
        if (currentToken_.type == TokenType::FUNCTION) {
            return functionDefinition();
        }
        if (currentToken_.type == TokenType::RETURN) {
            return returnStatement();
        }
        if (currentToken_.type == TokenType::IMPORT) {
            return importStatement();
        }
        if (currentToken_.type == TokenType::TRY) {
            return tryCatchStatement();
        }
        if (currentToken_.type == TokenType::THROW) {
            return throwStatement();
        }
        if (currentToken_.type == TokenType::GOTO) {
            return gotoStatement();
        }
        if (currentToken_.type == TokenType::LEFT_BRACE) {
            return blockStatement();
        }
        if (currentToken_.type == TokenType::CLASS) {
            return classDefinition();
        }
        if (currentToken_.type == TokenType::ENUM_CLASS) {
            return enumClassDefinition();
        }
        // 表达式语句
        return expressionStatement();
    }

    [[nodiscard]] auto ifStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::IF);
        consume(TokenType::LEFT_PARENTHESIS);
        auto condition = expression();
        consume(TokenType::RIGHT_PARENTHESIS);
        auto thenBranch = statement();
        std::shared_ptr<ASTNode> elseBranch = nullptr;
        if (currentToken_.type == TokenType::ELSE) {
            consume(TokenType::ELSE);
            elseBranch = statement();
        }
        return std::make_shared<IfStatement>(
            std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }

    [[nodiscard]] auto whileStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::WHILE);
        consume(TokenType::LEFT_PARENTHESIS);
        auto condition = expression();
        consume(TokenType::RIGHT_PARENTHESIS);
        auto body = statement();
        return std::make_shared<WhileStatement>(std::move(condition),
                                                std::move(body));
    }

    [[nodiscard]] auto forStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::FOR);
        consume(TokenType::LEFT_PARENTHESIS);
        auto initializer = statement();
        auto condition = expression();
        consume(TokenType::SEMICOLON);
        auto increment = statement();
        consume(TokenType::RIGHT_PARENTHESIS);
        auto body = statement();
        return std::make_shared<ForStatement>(
            std::move(initializer), std::move(condition), std::move(increment),
            std::move(body));
    }

    [[nodiscard]] auto switchStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::SWITCH);
        consume(TokenType::LEFT_PARENTHESIS);
        auto condition = expression();
        consume(TokenType::RIGHT_PARENTHESIS);
        consume(TokenType::LEFT_BRACE);
        std::vector<std::pair<std::shared_ptr<ASTNode>,
                              std::shared_ptr<BlockStatement>>>
            cases;
        while (currentToken_.type == TokenType::CASE) {
            consume(TokenType::CASE);
            auto caseValue = expression();
            consume(TokenType::COLON);
            auto caseBlock = blockStatement();
            cases.emplace_back(std::move(caseValue), std::move(caseBlock));
        }
        consume(TokenType::RIGHT_BRACE);
        return std::make_shared<SwitchStatement>(std::move(condition),
                                                 std::move(cases));
    }

    [[nodiscard]] auto functionDefinition() -> std::shared_ptr<ASTNode> {
        consume(TokenType::FUNCTION);
        std::string name = currentToken_.value;  // Function name
        consume(TokenType::IDENTIFIER);
        consume(TokenType::LEFT_PARENTHESIS);
        std::vector<std::string> params;

        if (currentToken_.type != TokenType::RIGHT_PARENTHESIS) {
            do {
                if (currentToken_.type != TokenType::IDENTIFIER) {
                    THROW_RUNTIME_ERROR("Expected parameter name");
                }
                params.push_back(currentToken_.value);
                consume(TokenType::IDENTIFIER);
                if (currentToken_.type == TokenType::COMMA) {
                    consume(TokenType::COMMA);
                } else {
                    break;
                }
            } while (true);
        }

        consume(TokenType::RIGHT_PARENTHESIS);
        auto bodyNode = blockStatement();
        return std::make_shared<FunctionDef>(name, std::move(params), bodyNode);
    }

    [[nodiscard]] auto classDefinition() -> std::shared_ptr<ASTNode> {
        consume(TokenType::CLASS);
        std::string className = currentToken_.value;
        consume(TokenType::IDENTIFIER);
        consume(TokenType::LEFT_BRACE);
        std::unordered_map<std::string, std::shared_ptr<ASTNode>> members;
        while (currentToken_.type != TokenType::RIGHT_BRACE) {
            if (currentToken_.type == TokenType::FUNCTION) {
                auto funcDef = functionDefinition();
                auto funcName =
                    std::dynamic_pointer_cast<FunctionDef>(funcDef)->getName();
                members[funcName] = std::move(funcDef);
            } else if (currentToken_.type == TokenType::IDENTIFIER) {
                std::string varName = currentToken_.value;
                consume(TokenType::IDENTIFIER);
                consume(TokenType::ASSIGNMENT);
                auto value = expression();
                consume(TokenType::SEMICOLON);
                members[varName] = std::make_shared<Assignment>(
                    std::make_shared<Identifier>(varName), std::move(value));
            } else {
                THROW_RUNTIME_ERROR("Unexpected token in class definition");
            }
        }
        consume(TokenType::RIGHT_BRACE);
        return std::make_shared<ClassDefinition>(className, std::move(members));
    }

    [[nodiscard]] auto enumClassDefinition() -> std::shared_ptr<ASTNode> {
        consume(TokenType::ENUM_CLASS);
        std::string enumName = currentToken_.value;
        consume(TokenType::IDENTIFIER);
        consume(TokenType::LEFT_BRACE);
        std::vector<std::string> enumerators;
        while (currentToken_.type != TokenType::RIGHT_BRACE) {
            enumerators.push_back(currentToken_.value);
            consume(TokenType::IDENTIFIER);
            if (currentToken_.type == TokenType::COMMA) {
                consume(TokenType::COMMA);
            } else {
                break;
            }
        }
        consume(TokenType::RIGHT_BRACE);
        return std::make_shared<EnumClassDefinition>(enumName,
                                                     std::move(enumerators));
    }

    [[nodiscard]] auto returnStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::RETURN);
        auto value = expression();
        consume(TokenType::SEMICOLON);
        return std::make_shared<ReturnStatement>(std::move(value));
    }

    [[nodiscard]] auto importStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::IMPORT);
        std::string moduleName = currentToken_.value;
        consume(TokenType::IDENTIFIER);
        std::string alias;
        if (currentToken_.type == TokenType::AS) {
            consume(TokenType::AS);
            alias = currentToken_.value;
            consume(TokenType::IDENTIFIER);
        } else {
            alias = moduleName;
        }
        consume(TokenType::SEMICOLON);
        return std::make_shared<ImportStatement>(moduleName, alias);
    }

    [[nodiscard]] auto tryCatchStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::TRY);
        auto tryBlock = blockStatement();
        consume(TokenType::CATCH);
        auto catchBlock = blockStatement();
        return std::make_shared<TryCatchStatement>(std::move(tryBlock),
                                                   std::move(catchBlock));
    }

    [[nodiscard]] auto throwStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::THROW);
        auto expr = expression();
        consume(TokenType::SEMICOLON);
        return std::make_shared<ThrowStatement>(std::move(expr));
    }

    [[nodiscard]] auto gotoStatement() -> std::shared_ptr<ASTNode> {
        consume(TokenType::GOTO);
        std::string label = currentToken_.value;
        consume(TokenType::IDENTIFIER);
        consume(TokenType::SEMICOLON);
        return std::make_shared<GotoStatement>(label);
    }

    [[nodiscard]] auto blockStatement() -> std::shared_ptr<BlockStatement> {
        consume(TokenType::LEFT_BRACE);
        auto block = std::make_shared<BlockStatement>();
        while (currentToken_.type != TokenType::RIGHT_BRACE) {
            block->addStatement(statement());
        }
        consume(TokenType::RIGHT_BRACE);
        return block;
    }

    [[nodiscard]] auto expressionStatement() -> std::shared_ptr<ASTNode> {
        auto expr = expression();
        consume(TokenType::SEMICOLON);
        return std::make_shared<ExpressionStatement>(std::move(expr));
    }

    [[nodiscard]] auto logical() -> std::shared_ptr<ASTNode> {
        auto node = comparison();
        while (currentToken_.type == TokenType::AND ||
               currentToken_.type == TokenType::OR) {
            Token opToken = currentToken_;
            consume(currentToken_.type);
            auto right = comparison();
            node = std::make_shared<BinaryOp>(std::move(node), opToken,
                                              std::move(right));
        }
        return node;
    }

    [[nodiscard]] auto expression() -> std::shared_ptr<ASTNode> {
        return logical();
    }

    [[nodiscard]] auto comparison() -> std::shared_ptr<ASTNode> {
        auto node = additive();
        while (currentToken_.type == TokenType::GREATER ||
               currentToken_.type == TokenType::LESS ||
               currentToken_.type == TokenType::GREATER_EQUAL ||
               currentToken_.type == TokenType::LESS_EQUAL ||
               currentToken_.type == TokenType::EQUAL ||
               currentToken_.type == TokenType::NOT_EQUAL) {
            Token opToken = currentToken_;
            consume(currentToken_.type);
            auto right = additive();
            node = std::make_shared<BinaryOp>(std::move(node), opToken,
                                              std::move(right));
        }
        return node;
    }

    [[nodiscard]] auto additive() -> std::shared_ptr<ASTNode> {
        auto node = multiplicative();
        while (currentToken_.type == TokenType::PLUS ||
               currentToken_.type == TokenType::MINUS) {
            Token opToken = currentToken_;
            consume(currentToken_.type);
            node = std::make_shared<BinaryOp>(std::move(node), opToken,
                                              multiplicative());
        }
        return node;
    }

    [[nodiscard]] auto multiplicative() -> std::shared_ptr<ASTNode> {
        auto node = primary();
        while (currentToken_.type == TokenType::MULTIPLY ||
               currentToken_.type == TokenType::DIVIDE) {
            Token opToken = currentToken_;
            consume(currentToken_.type);
            node =
                std::make_shared<BinaryOp>(std::move(node), opToken, primary());
        }
        return node;
    }

    [[nodiscard]] auto primary() -> std::shared_ptr<ASTNode> {
        if (currentToken_.type == TokenType::NUMBER) {
            auto numNode = std::make_shared<Number>(currentToken_.value);
            consume(TokenType::NUMBER);
            return numNode;
        }
        if (currentToken_.type == TokenType::STRING) {
            auto strNode = std::make_shared<StringLiteral>(currentToken_.value);
            consume(TokenType::STRING);
            return strNode;
        }
        if (currentToken_.type == TokenType::IDENTIFIER) {
            std::string name = currentToken_.value;
            consume(TokenType::IDENTIFIER);
            if (currentToken_.type == TokenType::DOUBLE_COLON) {
                consume(TokenType::DOUBLE_COLON);
                std::string funcName = currentToken_.value;
                consume(TokenType::IDENTIFIER);
                if (currentToken_.type == TokenType::LEFT_PARENTHESIS) {
                    return parseFunctionCall(name + "::" + funcName);
                }
                THROW_RUNTIME_ERROR(
                    "Expected '(' after external function name");
            }
            if (currentToken_.type == TokenType::LEFT_PARENTHESIS) {
                // 函数调用
                return parseFunctionCall(name);
            }
            return std::make_shared<Identifier>(name);
        }
        if (currentToken_.type == TokenType::LEFT_PARENTHESIS) {
            consume(TokenType::LEFT_PARENTHESIS);
            auto node = expression();
            consume(TokenType::RIGHT_PARENTHESIS);
            return node;
        }
        THROW_RUNTIME_ERROR("Unexpected token in primary expression: " +
                            currentToken_.value);
    }

    [[nodiscard]] auto parseFunctionCall(const std::string& name)
        -> std::shared_ptr<FunctionCall> {
        consume(TokenType::LEFT_PARENTHESIS);
        std::vector<std::shared_ptr<ASTNode>> args;
        if (currentToken_.type != TokenType::RIGHT_PARENTHESIS) {
            do {
                args.push_back(expression());
                if (currentToken_.type == TokenType::COMMA) {
                    consume(TokenType::COMMA);
                } else {
                    break;
                }
            } while (true);
        }
        consume(TokenType::RIGHT_PARENTHESIS);
        return std::make_shared<FunctionCall>(name, std::move(args));
    }
};

// 处理返回值的异常类
struct ReturnException : public std::exception {
    std::variant<int, double, std::string> value;

    explicit ReturnException(std::variant<int, double, std::string> val)
        : value(std::move(val)) {}

    [[nodiscard]] auto what() const noexcept -> const char* override {
        return "Return statement executed";
    }
} ATOM_ALIGNAS(64);

// GotoException类
struct GotoException : public std::exception {
    std::string label;

    explicit GotoException(std::string lbl) : label(std::move(lbl)) {}

    [[nodiscard]] auto what() const noexcept -> const char* override {
        return "Goto statement executed";
    }
} ATOM_ALIGNAS(32);

// Interpreter类
class Interpreter::Impl {
public:
    Impl() {
        registerBuiltInFunctions();
        registerInternalModules();
    }

    void loadScript(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            THROW_RUNTIME_ERROR("Failed to open script file: " + filename);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string script = buffer.str();
        Lexer lexer(script);
        Parser parser(lexer);
        auto program = parser.parse();
        scripts_[filename] = program;
    }

    void loadScript(const std::string& filename, const std::string& script) {
        Lexer lexer(script);
        Parser parser(lexer);
        auto program = parser.parse();
        scripts_[filename] = program;
    }

    void loadScript(const std::string& filename,
                    std::shared_ptr<Program> program) {
        scripts_[filename] = std::move(program);
    }

    void unloadScript(const std::string& filename) {
        auto it = scripts_.find(filename);
        if (it != scripts_.end()) {
            scripts_.erase(it);
        }
    }

    void interpretScript(const std::string& filename) {
        auto it = scripts_.find(filename);
        if (it == scripts_.end()) {
            THROW_RUNTIME_ERROR("Script not loaded: " + filename);
        }
        interpret(it->second);
    }

    void interpret(const std::shared_ptr<Program>& ast) {
        // 首先收集所有函数定义
        for (const auto& stmt : ast->getStatements()) {
            if (auto* funcDef = dynamic_cast<FunctionDef*>(stmt.get())) {
                functions_[funcDef->getName()] =
                    std::dynamic_pointer_cast<FunctionDef>(funcDef->clone());
                LOG_F(INFO, "Function '{}' defined.", funcDef->getName());
            }
        }

        // 执行所有非函数的语句
        for (size_t i = 0; i < ast->getStatements().size(); ++i) {
            const auto& stmt = ast->getStatements()[i];
            if (dynamic_cast<FunctionDef*>(stmt.get()) == nullptr) {
                try {
                    execute(stmt.get());
                } catch (const GotoException& e) {
                    LOG_F(INFO,
                          "Goto exception caught, searching for label: {}",
                          e.label);
                    // 搜索标签
                    for (size_t j = 0; j < ast->getStatements().size(); ++j) {
                        if (auto* labelStmt = dynamic_cast<LabelStatement*>(
                                ast->getStatements()[j].get())) {
                            if (labelStmt->getLabel() == e.label) {
                                i = j;  // 跳转到标签位置
                                break;
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    printStackTrace();
                    throw;
                }
            }
        }
    }

    void registerFunction(const std::string& name,
                          std::shared_ptr<FunctionDef> func) {
        functions_[name] = std::move(func);
    }

    void registerGlobal(const std::string& name, int value) {
        globals_[name] = value;
    }

    void registerGlobal(const std::string& name, double value) {
        globals_[name] = value;
    }

    void registerGlobal(const std::string& name, const std::string& value) {
        globals_[name] = value;
    }

private:
    // 函数表
    std::unordered_map<std::string, std::shared_ptr<FunctionDef>> functions_;

    // 全局变量
    std::unordered_map<std::string, std::variant<int, double, std::string>>
        globals_;

    // 变量栈，用于函数调用的局部变量
    std::vector<
        std::unordered_map<std::string, std::variant<int, double, std::string>>>
        locals_;

    // 调用堆栈
    std::vector<std::string> callStack_;

    // 脚本表
    std::unordered_map<std::string, std::shared_ptr<Program>> scripts_;

    void printStackTrace() const {
        std::cerr << "Stack trace:" << std::endl;
        for (const auto& funcName : callStack_) {
            std::cerr << "  at " << funcName << std::endl;
        }
    }

    // 评估表达式
    [[nodiscard]] auto evaluate(ASTNode* node)
        -> std::variant<int, double, std::string> {
        if (auto* binaryOp = dynamic_cast<BinaryOp*>(node)) {
            auto left = evaluate(binaryOp->getLeft().get());
            auto right = evaluate(binaryOp->getRight().get());
            LOG_F(INFO, "Evaluating binary operation: {}",
                  tokenTypeToString(binaryOp->getOp().type));

            return evaluateBinaryOp(binaryOp->getOp().type, left, right);
        }
        if (auto* numberNode = dynamic_cast<Number*>(node)) {
            const auto& value = numberNode->getValue();
            if (value.contains('.')) {
                double val = std::stod(value);
                LOG_F(INFO, "Number literal (double): {}", val);
                return val;
            }
            int val = std::stoi(value);
            LOG_F(INFO, "Number literal (int): {}", val);
            return val;
        }
        if (auto* strNode = dynamic_cast<StringLiteral*>(node)) {
            return strNode->getValue();
        }
        if (auto* idNode = dynamic_cast<Identifier*>(node)) {
            auto val = getVariable(idNode->getName());
            return val;
        }
        if (auto* funcCall = dynamic_cast<FunctionCall*>(node)) {
            LOG_F(INFO, "Function call: {}", funcCall->getName());
            return callFunction(funcCall->getName(), funcCall->getArguments());
        }
        THROW_RUNTIME_ERROR("Unknown expression type");
    }

    // 执行语句
    void execute(ASTNode* node) {
        if (auto* exprStmt = dynamic_cast<ExpressionStatement*>(node)) {
            auto result = evaluate(exprStmt->getExpression().get());
        } else if (auto* assignStmt = dynamic_cast<Assignment*>(node)) {
            auto value = evaluate(assignStmt->getValue().get());
            setVariable(assignStmt->getIdentifier()->getName(), value);
        } else if (auto* ifStmt = dynamic_cast<IfStatement*>(node)) {
            auto condition = evaluate(ifStmt->getCondition().get());
            bool cond = variantToBool(condition);
            if (cond) {
                execute(ifStmt->getThenBranch().get());
            } else if (ifStmt->getElseBranch()) {
                execute(ifStmt->getElseBranch().get());
            }
        } else if (auto* whileStmt = dynamic_cast<WhileStatement*>(node)) {
            while (variantToBool(evaluate(whileStmt->getCondition().get()))) {
                LOG_F(INFO, "While loop condition true, executing body.");
                execute(whileStmt->getBody().get());
            }
            LOG_F(INFO, "While loop condition false, exiting loop.");
        } else if (auto* forStmt = dynamic_cast<ForStatement*>(node)) {
            for (execute(forStmt->getInitializer().get());
                 variantToBool(evaluate(forStmt->getCondition().get()));
                 execute(forStmt->getIncrement().get())) {
                LOG_F(INFO, "For loop condition true, executing body.");
                execute(forStmt->getBody().get());
            }
            LOG_F(INFO, "For loop condition false, exiting loop.");
        } else if (auto* switchStmt = dynamic_cast<SwitchStatement*>(node)) {
            auto condition = evaluate(switchStmt->getCondition().get());
            for (const auto& [caseValue, caseBlock] : switchStmt->getCases()) {
                if (evaluate(caseValue.get()) == condition) {
                    execute(caseBlock.get());
                    break;
                }
            }
        } else if (auto* blockStmt = dynamic_cast<BlockStatement*>(node)) {
            for (const auto& stmt : blockStmt->getStatements()) {
                execute(stmt.get());
            }
        } else if (auto* returnStmt = dynamic_cast<ReturnStatement*>(node)) {
            auto value = evaluate(returnStmt->getValue().get());
            throw ReturnException(value);
        } else if (auto* importStmt = dynamic_cast<ImportStatement*>(node)) {
            importModule(importStmt->getModuleName(), importStmt->getAlias());
        } else if (auto* tryCatchStmt =
                       dynamic_cast<TryCatchStatement*>(node)) {
            try {
                execute(tryCatchStmt->getTryBlock().get());
            } catch (const std::exception& e) {
                execute(tryCatchStmt->getCatchBlock().get());
            }
        } else if (auto* throwStmt = dynamic_cast<ThrowStatement*>(node)) {
            auto value = evaluate(throwStmt->getExpression().get());
            THROW_RUNTIME_ERROR(variantToString(value));
        } else if (auto* gotoStmt = dynamic_cast<GotoStatement*>(node)) {
            auto label = gotoStmt->getLabel();
            throw GotoException(label);
        } else if (auto* labelStmt = dynamic_cast<LabelStatement*>(node)) {
            // 标签语句不执行任何操作
        } else if (auto* classDef = dynamic_cast<ClassDefinition*>(node)) {
            for (const auto& [name, member] : classDef->getMembers()) {
                execute(member.get());
            }
        } else if (auto* enumClassDef =
                       dynamic_cast<EnumClassDefinition*>(node)) {
            // for (const auto& enumerator : enumClassDef->getEnumerators()) {
            // }
        } else {
            THROW_RUNTIME_ERROR("Unknown statement type");
        }
    }

    void registerBuiltInFunctions() {
        functions_["print"] = std::make_shared<FunctionDef>(
            "print", std::vector<std::string>{"message"},
            std::make_shared<BlockStatement>());

        functions_["len"] = std::make_shared<FunctionDef>(
            "len", std::vector<std::string>{"value"},
            std::make_shared<BlockStatement>());

        functions_["toInt"] = std::make_shared<FunctionDef>(
            "toInt", std::vector<std::string>{"value"},
            std::make_shared<BlockStatement>());

        functions_["toDouble"] = std::make_shared<FunctionDef>(
            "toDouble", std::vector<std::string>{"value"},
            std::make_shared<BlockStatement>());
    }

    void registerInternalModules() {
        // 注册多线程模块
        functions_["thread::create"] = std::make_shared<FunctionDef>(
            "thread::create", std::vector<std::string>{"function"},
            std::make_shared<BlockStatement>());
    }

    void callBuiltInFunction(
        const std::string& name,
        const std::vector<std::shared_ptr<ASTNode>>& args) {
        if (name == "print") {
            if (args.size() != 1) {
                THROW_RUNTIME_ERROR("print function expects 1 argument");
            }
            auto message = evaluate(args[0].get());
            std::visit([](auto&& arg) { std::cout << arg << std::endl; },
                       message);
        } else if (name == "len") {
            if (args.size() != 1) {
                THROW_RUNTIME_ERROR("len function expects 1 argument");
            }
            auto value = evaluate(args[0].get());
            if (std::holds_alternative<std::string>(value)) {
                std::cout << std::get<std::string>(value).size() << std::endl;
            } else {
                THROW_RUNTIME_ERROR("len function expects a string argument");
            }
        } else if (name == "toInt") {
            if (args.size() != 1) {
                THROW_RUNTIME_ERROR("toInt function expects 1 argument");
            }
            auto value = evaluate(args[0].get());
            if (std::holds_alternative<std::string>(value)) {
                std::cout << std::stoi(std::get<std::string>(value))
                          << std::endl;
            } else if (std::holds_alternative<double>(value)) {
                std::cout << static_cast<int>(std::get<double>(value))
                          << std::endl;
            } else {
                THROW_RUNTIME_ERROR(
                    "toInt function expects a string or double argument");
            }
        } else if (name == "toDouble") {
            if (args.size() != 1) {
                THROW_RUNTIME_ERROR("toDouble function expects 1 argument");
            }
            auto value = evaluate(args[0].get());
            if (std::holds_alternative<std::string>(value)) {
                std::cout << std::stod(std::get<std::string>(value))
                          << std::endl;
            } else if (std::holds_alternative<int>(value)) {
                std::cout << static_cast<double>(std::get<int>(value))
                          << std::endl;
            } else {
                THROW_RUNTIME_ERROR(
                    "toDouble function expects a string or int argument");
            }
        } else if (name == "thread::create") {
            if (args.size() != 1) {
                THROW_RUNTIME_ERROR(
                    "thread::create function expects 1 argument");
            }
            auto funcName = std::get<std::string>(evaluate(args[0].get()));
            auto funcIt = functions_.find(funcName);
            if (funcIt == functions_.end()) {
                THROW_RUNTIME_ERROR("Undefined function: " + funcName);
            }
            auto* func = funcIt->second.get();
            std::thread([this, func]() {
                try {
                    for (const auto& stmt : func->getBody()->getStatements()) {
                        execute(stmt.get());
                    }
                } catch (const ReturnException&) {
                    // 忽略返回值
                }
            }).detach();
        } else {
            THROW_RUNTIME_ERROR("Unknown built-in function: " + name);
        }
    }

    // 调用函数
    // 调用函数
    [[nodiscard]] auto callFunction(
        const std::string& name,
        const std::vector<std::shared_ptr<ASTNode>>& args)
        -> std::variant<int, double, std::string> {
        // 检查是否是内置函数
        if (functions_.find(name) == functions_.end()) {
            callBuiltInFunction(name, args);
            return 0;
        }
        // 查找函数定义
        auto funcIt = functions_.find(name);
        if (funcIt == functions_.end()) {
            return callExternalFunction(name, args);
        }
        auto* func = funcIt->second.get();

        if (args.size() != func->getParams().size()) {
            THROW_RUNTIME_ERROR("Function " + name + " expects " +
                                std::to_string(func->getParams().size()) +
                                " arguments, got " +
                                std::to_string(args.size()));
        }

        LOG_F(INFO, "Calling function '{}'", name);

        // 创建新的局部作用域
        locals_.emplace_back();

        // 绑定参数
        for (size_t idx = 0; idx < args.size(); ++idx) {
            auto argValue = evaluate(args[idx].get());
            locals_.back()[func->getParams()[idx]] = argValue;
            LOG_F(INFO, "Function parameter '{}' assigned value: {}",
                  func->getParams()[idx], variantToString(argValue));
        }

        // 添加到调用堆栈
        callStack_.push_back(name);

        try {
            // 执行函数体
            for (const auto& stmt : func->getBody()->getStatements()) {
                execute(stmt.get());
            }
        } catch (const ReturnException& ret) {
            // 弹出局部作用域
            locals_.pop_back();
            // 从调用堆栈中移除
            callStack_.pop_back();
            LOG_F(INFO, "Function '{}' returned value: {}", name,
                  variantToString(ret.value));
            return ret.value;
        } catch (const std::exception& e) {
            // 从调用堆栈中移除
            callStack_.pop_back();
            throw;
        }

        // 如果函数没有返回语句，默认返回0
        locals_.pop_back();
        // 从调用堆栈中移除
        callStack_.pop_back();
        return 0;
    }

    [[nodiscard]] auto callExternalFunction(
        const std::string& name,
        const std::vector<std::shared_ptr<ASTNode>>& args)
        -> std::variant<int, double, std::string> {
        void* funcPtr = dlsym(RTLD_DEFAULT, name.c_str());
        if (funcPtr == nullptr) {
            THROW_RUNTIME_ERROR("Undefined external function: " + name);
        }

        // 准备libffi调用
        ffi_cif cif;
        std::vector<ffi_type*> argTypes(args.size());
        std::vector<void*> argValues(args.size());
        std::vector<double> doubleStorage(args.size());
        std::vector<std::string> stringStorage(args.size());
        std::vector<const char*> cStringStorage(args.size());

        for (size_t i = 0; i < args.size(); ++i) {
            auto argValue = evaluate(args[i].get());
            if (std::holds_alternative<int>(argValue)) {
                doubleStorage[i] = static_cast<double>(std::get<int>(argValue));
                argTypes[i] = &ffi_type_double;
                argValues[i] = &doubleStorage[i];
            } else if (std::holds_alternative<double>(argValue)) {
                doubleStorage[i] = std::get<double>(argValue);
                argTypes[i] = &ffi_type_double;
                argValues[i] = &doubleStorage[i];
            } else if (std::holds_alternative<std::string>(argValue)) {
                stringStorage[i] = std::get<std::string>(argValue);
                cStringStorage[i] = stringStorage[i].c_str();
                argTypes[i] = &ffi_type_pointer;
                argValues[i] = &cStringStorage[i];
            } else {
                THROW_RUNTIME_ERROR(
                    "Unsupported argument type for external function");
            }
        }

        // 假设返回类型为double
        ffi_type* returnType = &ffi_type_double;
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), returnType,
                         argTypes.data()) != FFI_OK) {
            THROW_RUNTIME_ERROR("Failed to prepare CIF for external function");
        }

        ffi_arg result;
        ffi_call(&cif, FFI_FN(funcPtr), &result, argValues.data());

        // 尝试将结果转换为double
        double doubleResult = *reinterpret_cast<double*>(&result);
        if (doubleResult == static_cast<int>(doubleResult)) {
            return static_cast<int>(doubleResult);
        }
        return doubleResult;
    }

    // 导入模块
    void importModule(const std::string& moduleName, const std::string& alias) {
        std::string fileName = "./" + moduleName + ".so";
        if (!fs::exists(fileName)) {
            THROW_RUNTIME_ERROR("Module not found: " + fileName);
        }
        if (dlopen(fileName.c_str(), RTLD_NOLOAD) != nullptr) {
            LOG_F(INFO, "Module '{}' already loaded.", moduleName);
            return;
        }
        void* handle = dlopen(fileName.c_str(), RTLD_LAZY);
        if (handle == nullptr) {
            THROW_RUNTIME_ERROR("Failed to load module: " + fileName);
        }
        LOG_F(INFO, "Module '{}' loaded.", moduleName);

        // 导入模块中的函数
        std::vector<std::string> importedFunctions;
        importedFunctions.reserve(functions_.size());
        for (const auto& [name, func] : functions_) {
            importedFunctions.push_back(name);
        }
        for (const auto& funcName : importedFunctions) {
            auto funcIt = functions_.find(funcName);
            if (funcIt == functions_.end()) {
                THROW_RUNTIME_ERROR("Undefined function: " + funcName);
            }
            auto* func = funcIt->second.get();
            if (func->getName() == funcName) {
                functions_["{}::{}"_fmt(alias, funcName)] = funcIt->second;
                LOG_F(INFO, "Function '{}' imported from module '{}' as '{}'",
                      funcName, moduleName, alias + "::" + funcName);
            }
        }
    }

    // 获取变量值
    [[nodiscard]] auto getVariable(const std::string& name)
        -> std::variant<int, double, std::string> {
        // 优先从局部作用域查找
        for (auto& local : std::ranges::reverse_view(locals_)) {
            auto varIt = local.find(name);
            if (varIt != local.end()) {
                return varIt->second;
            }
        }
        // 从全局作用域查找
        auto it = globals_.find(name);
        if (it != globals_.end()) {
            return it->second;
        }
        THROW_RUNTIME_ERROR("Variable not found: " + name);
    }

    // 设置变量值
    void setVariable(const std::string& name,
                     const std::variant<int, double, std::string>& value) {
        // 如果在局部作用域中存在，赋值到最内层的局部作用域
        for (auto& local : std::ranges::reverse_view(locals_)) {
            if (local.find(name) != local.end()) {
                local[name] = value;
                return;
            }
        }
        // 否则，赋值到全局作用域
        globals_[name] = value;
    }

    // 转换TokenType为字符串
    static auto tokenTypeToString(TokenType type) -> std::string {
        switch (type) {
            case TokenType::PLUS:
                return "+";
            case TokenType::MINUS:
                return "-";
            case TokenType::MULTIPLY:
                return "*";
            case TokenType::DIVIDE:
                return "/";
            case TokenType::GREATER:
                return ">";
            case TokenType::LESS:
                return "<";
            case TokenType::GREATER_EQUAL:
                return ">=";
            case TokenType::LESS_EQUAL:
                return "<=";
            case TokenType::EQUAL:
                return "==";
            case TokenType::NOT_EQUAL:
                return "!=";
            default:
                return "unknown";
        }
    }

    // 将variant类型转换为字符串
    static auto variantToString(
        const std::variant<int, double, std::string>& var) -> std::string {
        if (std::holds_alternative<int>(var)) {
            return std::to_string(std::get<int>(var));
        }
        if (std::holds_alternative<double>(var)) {
            return std::to_string(std::get<double>(var));
        }
        if (std::holds_alternative<std::string>(var)) {
            return "\"" + std::get<std::string>(var) + "\"";
        }
        return "unknown";
    }

    // 将variant类型转换为布尔值
    static auto variantToBool(const std::variant<int, double, std::string>& var)
        -> bool {
        if (std::holds_alternative<int>(var)) {
            return std::get<int>(var) != 0;
        }
        if (std::holds_alternative<double>(var)) {
            return std::get<double>(var) != 0.0;
        }
        if (std::holds_alternative<std::string>(var)) {
            return !std::get<std::string>(var).empty();
        }
        return false;
    }

    // 评估二元操作
    static auto evaluateBinaryOp(
        TokenType opType, const std::variant<int, double, std::string>& left,
        const std::variant<int, double, std::string>& right)
        -> std::variant<int, double, std::string> {
        // 数字运算
        if ((std::holds_alternative<int>(left) ||
             std::holds_alternative<double>(left)) &&
            (std::holds_alternative<int>(right) ||
             std::holds_alternative<double>(right))) {
            double leftVal = std::holds_alternative<int>(left)
                                 ? static_cast<double>(std::get<int>(left))
                                 : std::get<double>(left);
            double rightVal = std::holds_alternative<int>(right)
                                  ? static_cast<double>(std::get<int>(right))
                                  : std::get<double>(right);
            switch (opType) {
                case TokenType::PLUS:
                    return leftVal + rightVal;
                case TokenType::MINUS:
                    return leftVal - rightVal;
                case TokenType::MULTIPLY:
                    return leftVal * rightVal;
                case TokenType::DIVIDE:
                    if (rightVal == 0.0) {
                        THROW_RUNTIME_ERROR("Division by zero");
                    }
                    return leftVal / rightVal;
                case TokenType::GREATER:
                    return leftVal > rightVal ? 1 : 0;
                case TokenType::LESS:
                    return leftVal < rightVal ? 1 : 0;
                case TokenType::GREATER_EQUAL:
                    return leftVal >= rightVal ? 1 : 0;
                case TokenType::LESS_EQUAL:
                    return leftVal <= rightVal ? 1 : 0;
                case TokenType::EQUAL:
                    return (leftVal == rightVal) ? 1 : 0;
                case TokenType::NOT_EQUAL:
                    return (leftVal != rightVal) ? 1 : 0;
                default:
                    THROW_RUNTIME_ERROR("Unknown binary operator");
            }
        }

        // 字符串连接
        if (opType == TokenType::PLUS) {
            if (std::holds_alternative<std::string>(left) &&
                std::holds_alternative<std::string>(right)) {
                return std::get<std::string>(left) +
                       std::get<std::string>(right);
            }
            if (std::holds_alternative<std::string>(left)) {
                return std::get<std::string>(left) +
                       std::visit(
                           [](auto&& arg) -> std::string {
                               if constexpr (std::is_same_v<
                                                 std::decay_t<decltype(arg)>,
                                                 int> ||
                                             std::is_same_v<
                                                 std::decay_t<decltype(arg)>,
                                                 double>) {
                                   return std::to_string(arg);
                               } else {
                                   THROW_RUNTIME_ERROR(
                                       "Unsupported type for string "
                                       "concatenation");
                               }
                           },
                           right);
            }
            if (std::holds_alternative<std::string>(right)) {
                return std::visit(
                           [](auto&& arg) -> std::string {
                               if constexpr (std::is_same_v<
                                                 std::decay_t<decltype(arg)>,
                                                 int> ||
                                             std::is_same_v<
                                                 std::decay_t<decltype(arg)>,
                                                 double>) {
                                   return std::to_string(arg);
                               } else {
                                   THROW_RUNTIME_ERROR(
                                       "Unsupported type for string "
                                       "concatenation");
                               }
                           },
                           left) +
                       std::get<std::string>(right);
            }
        }

        // 比较字符串
        if ((opType == TokenType::GREATER || opType == TokenType::LESS ||
             opType == TokenType::GREATER_EQUAL ||
             opType == TokenType::LESS_EQUAL || opType == TokenType::EQUAL ||
             opType == TokenType::NOT_EQUAL) &&
            std::holds_alternative<std::string>(left) &&
            std::holds_alternative<std::string>(right)) {
            const auto& leftStr = std::get<std::string>(left);
            const auto& rightStr = std::get<std::string>(right);
            switch (opType) {
                case TokenType::GREATER:
                    return leftStr > rightStr ? 1 : 0;
                case TokenType::LESS:
                    return leftStr < rightStr ? 1 : 0;
                case TokenType::GREATER_EQUAL:
                    return leftStr >= rightStr ? 1 : 0;
                case TokenType::LESS_EQUAL:
                    return leftStr <= rightStr ? 1 : 0;
                case TokenType::EQUAL:
                    return (leftStr == rightStr) ? 1 : 0;
                case TokenType::NOT_EQUAL:
                    return (leftStr != rightStr) ? 1 : 0;
                default:
                    THROW_RUNTIME_ERROR("Unknown binary operator for strings");
            }
        }

        THROW_RUNTIME_ERROR(
            "Unsupported binary operation between different types");
    }
};

Interpreter::Interpreter() : impl_(std::make_unique<Impl>()) {}

auto Interpreter::createShared() -> std::shared_ptr<Interpreter> {
    return std::make_shared<Interpreter>();
}

void Interpreter::loadScript(const std::string& filename) {
    impl_->loadScript(filename);
}

void Interpreter::interpretScript(const std::string& filename) {
    impl_->interpretScript(filename);
}

void Interpreter::interpret(const std::shared_ptr<Program>& ast) {
    impl_->interpret(ast);
};

}  // namespace lithium
