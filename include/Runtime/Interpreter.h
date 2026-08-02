#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "../Compiler/Lexer.h"
#include "../Compiler/Operator.h"
#include "../Type/Primative.h"

namespace Dyno::Runtime {

    class Interpreter {
        public:
            using Token = Dyno::Compiler::Token;
            using Token_t = Dyno::Compiler::Token_t;

            struct Object;
            using ObjectPtr = std::shared_ptr<Object>;
            using Value = std::variant<std::monostate, bool, std::int64_t, double, std::string, ObjectPtr>;

            struct Object {
                std::string typeName;
                std::unordered_map<std::string, Value> fields;
            };

            explicit Interpreter(const std::string& sourcePath, const Dyno::Util::Configure& conf)
                : conf(conf), lexer(sourcePath, conf) {
                this->lexer.tokensize();
                this->tokens = this->lexer.getTokens();
                this->parseFunctions();
            }

            void run() {
                (void)this->invoke("main", {});
            }

            void dumpCompiledAsm(std::ostream& out) const {
                out << "; Dynoscript compiled ASM" << '\n';
                this->dumpClasses(out);

                for (const auto& [name, function] : this->functions) {
                    out << function.header << " {" << '\n';
                    for (const auto& statement : this->splitStatements(function.body)) {
                        const std::string asmLine = this->statementToAsm(statement);
                        if (!asmLine.empty()) {
                            out << "  " << asmLine << '\n';
                        }
                    }
                    out << "}" << '\n' << '\n';
                }
            }

        private:
            struct FunctionDef {
                std::string name;
                std::string returnType;
                std::string header;
                std::vector<std::string> parameters;
                std::vector<Token> body;
            };

            struct Scope {
                std::unordered_map<std::string, Value> values;
                std::unordered_map<std::string, std::string> types;
            };

            struct ExecResult {
                bool returned = false;
                Value value = std::monostate{};
            };

            Dyno::Util::Configure conf;
            Dyno::Compiler::Lexer lexer;
            std::vector<Token> tokens;
            std::unordered_map<std::string, FunctionDef> functions;

            static std::string stripQuotes(const std::string& text) {
                if (text.size() >= 2 && ((text.front() == '"' && text.back() == '"') || (text.front() == '\'' && text.back() == '\''))) {
                    std::string result = text.substr(1, text.size() - 2);
                    std::string unescaped;
                    unescaped.reserve(result.size());
                    for (std::size_t i = 0; i < result.size(); ++i) {
                        if (result[i] == '\\' && i + 1 < result.size()) {
                            char next = result[++i];
                            switch (next) {
                                case 'n': unescaped.push_back('\n'); break;
                                case 'r': unescaped.push_back('\r'); break;
                                case 't': unescaped.push_back('\t'); break;
                                case '\\': unescaped.push_back('\\'); break;
                                case '"': unescaped.push_back('"'); break;
                                case '\'': unescaped.push_back('\''); break;
                                default: unescaped.push_back(next); break;
                            }
                        } else {
                            unescaped.push_back(result[i]);
                        }
                    }
                    return unescaped;
                }
                return text;
            }

            static std::string joinTokens(const std::vector<Token>& items, std::size_t start, std::size_t end) {
                std::string result;
                for (std::size_t i = start; i < end; ++i) {
                    result += items[i].literal;
                }
                return result;
            }

            static std::string canonicalTypeName(const std::string& typeName) {
                std::string normalized = typeName;
                while (normalized.rfind("std::", 0) == 0) {
                    normalized.erase(0, 5);
                }

                if (normalized == "string") {
                    return "str";
                }

                if (normalized == "sint") {
                    return "sint64";
                }

                return normalized;
            }

            static bool isIntegerType(const std::string& typeName) {
                const std::string normalized = canonicalTypeName(typeName);
                return normalized.rfind("int", 0) == 0 || normalized.rfind("uint", 0) == 0 || normalized.rfind("sint", 0) == 0 || normalized.rfind("byte", 0) == 0 || normalized.rfind("char", 0) == 0;
            }

            static bool isFloatType(const std::string& typeName) {
                const std::string normalized = canonicalTypeName(typeName);
                return normalized.rfind("float", 0) == 0 || normalized.find("double") != std::string::npos;
            }

            static bool isStringType(const std::string& typeName) {
                const std::string normalized = canonicalTypeName(typeName);
                return normalized == "str" || normalized == "string";
            }

            static bool isBoolType(const std::string& typeName) {
                const std::string normalized = canonicalTypeName(typeName);
                return normalized == "bool";
            }

            static bool isVoidType(const std::string& typeName) {
                const std::string normalized = canonicalTypeName(typeName);
                return normalized.empty() || normalized == "void";
            }

            static bool isNumericLiteral(const std::string& text) {
                return !text.empty() && (std::isdigit(static_cast<unsigned char>(text.front())) || ((text.front() == '-' || text.front() == '+') && text.size() > 1 && std::isdigit(static_cast<unsigned char>(text[1]))));
            }

            static double valueToDouble(const Value& value) {
                if (std::holds_alternative<std::int64_t>(value)) {
                    return static_cast<double>(std::get<std::int64_t>(value));
                }
                if (std::holds_alternative<double>(value)) {
                    return std::get<double>(value);
                }
                if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value) ? 1.0 : 0.0;
                }
                if (std::holds_alternative<std::string>(value)) {
                    return std::stod(std::get<std::string>(value));
                }
                return 0.0;
            }

            static std::int64_t valueToInt(const Value& value) {
                if (std::holds_alternative<std::int64_t>(value)) {
                    return std::get<std::int64_t>(value);
                }
                if (std::holds_alternative<double>(value)) {
                    return static_cast<std::int64_t>(std::get<double>(value));
                }
                if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value) ? 1 : 0;
                }
                if (std::holds_alternative<std::string>(value)) {
                    return std::stoll(std::get<std::string>(value));
                }
                return 0;
            }

            static std::string valueToString(const Value& value) {
                if (std::holds_alternative<std::monostate>(value)) {
                    return "null";
                }
                if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value) ? "true" : "false";
                }
                if (std::holds_alternative<std::int64_t>(value)) {
                    return std::to_string(std::get<std::int64_t>(value));
                }
                if (std::holds_alternative<double>(value)) {
                    return std::to_string(std::get<double>(value));
                }
                if (std::holds_alternative<std::string>(value)) {
                    return std::get<std::string>(value);
                }

                const auto& object = std::get<ObjectPtr>(value);
                return object ? "<" + object->typeName + ">" : "<null-object>";
            }

            static bool valueTruthy(const Value& value) {
                if (std::holds_alternative<std::monostate>(value)) {
                    return false;
                }
                if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value);
                }
                if (std::holds_alternative<std::int64_t>(value)) {
                    return std::get<std::int64_t>(value) != 0;
                }
                if (std::holds_alternative<double>(value)) {
                    return std::get<double>(value) != 0.0;
                }
                if (std::holds_alternative<std::string>(value)) {
                    return !std::get<std::string>(value).empty();
                }
                return std::get<ObjectPtr>(value) != nullptr;
            }

            static Value defaultValueForType(const std::string& typeName) {
                const std::string normalized = canonicalTypeName(typeName);

                if (isVoidType(normalized)) {
                    return std::monostate{};
                }
                if (isBoolType(normalized)) {
                    return false;
                }
                if (isStringType(normalized)) {
                    return std::string{};
                }
                if (isFloatType(normalized)) {
                    return 0.0;
                }
                if (isIntegerType(normalized)) {
                    return static_cast<std::int64_t>(0);
                }

                return std::make_shared<Object>(Object{normalized, {}});
            }

            static Value coerceToType(const std::string& typeName, Value value) {
                const std::string normalized = canonicalTypeName(typeName);

                if (isVoidType(normalized)) {
                    return std::monostate{};
                }
                if (isBoolType(normalized)) {
                    return valueTruthy(value);
                }
                if (isStringType(normalized)) {
                    return valueToString(value);
                }
                if (isFloatType(normalized)) {
                    return valueToDouble(value);
                }
                if (isIntegerType(normalized)) {
                    return valueToInt(value);
                }
                return value;
            }

            static std::size_t findMatching(const std::vector<Token>& items, std::size_t openIndex, const std::string& openLiteral, const std::string& closeLiteral) {
                std::size_t depth = 0;
                for (std::size_t i = openIndex; i < items.size(); ++i) {
                    if (items[i].literal == openLiteral) {
                        ++depth;
                    } else if (items[i].literal == closeLiteral) {
                        if (depth == 0) {
                            throw std::runtime_error("Unbalanced tokens while searching for matching delimiter.");
                        }
                        --depth;
                        if (depth == 0) {
                            return i;
                        }
                    }
                }
                throw std::runtime_error("Missing closing delimiter in Dynoscript source.");
            }

            static void skipTemplateArguments(const std::vector<Token>& items, std::size_t& pos) {
                if (pos >= items.size() || items[pos].literal != "<") {
                    return;
                }

                std::size_t depth = 0;
                while (pos < items.size()) {
                    if (items[pos].literal == "<") {
                        ++depth;
                    } else if (items[pos].literal == ">") {
                        if (depth == 0) {
                            break;
                        }
                        --depth;
                        if (depth == 0) {
                            ++pos;
                            return;
                        }
                    }
                    ++pos;
                }
                throw std::runtime_error("Unterminated template argument list.");
            }

            static bool isDeclarationLike(const std::vector<Token>& items) {
                if (items.size() < 2) {
                    return false;
                }

                for (const auto& token : items) {
                    if (token.literal == "(" || token.literal == ")" || token.literal == "{" || token.literal == "}" || token.type == Token_t::OPERATOR) {
                        return false;
                    }
                }

                std::size_t lastIdentifier = items.size();
                for (std::size_t i = items.size(); i > 0; --i) {
                    if (items[i - 1].type == Token_t::IDENTIFIER) {
                        lastIdentifier = i - 1;
                        break;
                    }
                }

                if (lastIdentifier == items.size() || lastIdentifier == 0) {
                    return false;
                }

                if (items[lastIdentifier - 1].literal == "." || items[lastIdentifier - 1].literal == "::") {
                    return false;
                }

                return true;
            }

            static std::string declarationName(const std::vector<Token>& items) {
                for (std::size_t i = items.size(); i > 0; --i) {
                    if (items[i - 1].type == Token_t::IDENTIFIER) {
                        return items[i - 1].literal;
                    }
                }
                return {};
            }

            static std::string declarationType(const std::vector<Token>& items) {
                if (items.size() < 2) {
                    return {};
                }

                std::size_t lastIdentifier = 0;
                for (std::size_t i = items.size(); i > 0; --i) {
                    if (items[i - 1].type == Token_t::IDENTIFIER) {
                        lastIdentifier = i - 1;
                        break;
                    }
                }

                return joinTokens(items, 0, lastIdentifier);
            }

            static std::size_t findTopLevelAssignment(const std::vector<Token>& items) {
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;
                for (std::size_t i = 0; i < items.size(); ++i) {
                    const auto& token = items[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "{") {
                        ++brace;
                    } else if (token.literal == "}") {
                        if (brace > 0) {
                            --brace;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    } else if (token.literal == "=" && paren == 0 && brace == 0 && bracket == 0) {
                        return i;
                    }
                }
                return items.size();
            }

            static std::size_t findTopLevelDelimited(const std::vector<Token>& items, std::size_t start, std::size_t end, const std::string& delimiter) {
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;

                for (std::size_t i = start; i < end; ++i) {
                    const auto& token = items[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "{") {
                        ++brace;
                    } else if (token.literal == "}") {
                        if (brace > 0) {
                            --brace;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    } else if (token.literal == delimiter && paren == 0 && brace == 0 && bracket == 0) {
                        return i;
                    }
                }

                return end;
            }

            struct AttachedBlock {
                bool hasBlock = false;
                std::vector<Token> header;
                std::vector<Token> body;
            };

            static AttachedBlock splitAttachedBlock(const std::vector<Token>& statement) {
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;

                for (std::size_t i = 0; i < statement.size(); ++i) {
                    const auto& token = statement[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    } else if (token.literal == "{" && paren == 0 && brace == 0 && bracket == 0) {
                        std::size_t close = findMatching(statement, i, "{", "}");
                        if (close + 1 != statement.size()) {
                            return {};
                        }

                        AttachedBlock block;
                        block.hasBlock = true;
                        block.header.assign(statement.begin(), statement.begin() + static_cast<std::ptrdiff_t>(i));
                        block.body.assign(statement.begin() + static_cast<std::ptrdiff_t>(i + 1), statement.begin() + static_cast<std::ptrdiff_t>(close));
                        return block;
                    }
                }

                return {};
            }

            void parseFunctions() {
                for (std::size_t i = 0; i < this->tokens.size();) {
                    if (this->tokens[i].literal == "Function" && i + 1 < this->tokens.size() && this->tokens[i + 1].literal == "<") {
                        std::size_t returnOpen = i + 1;
                        std::size_t returnClose = findMatching(this->tokens, returnOpen, "<", ">");
                        std::string returnType = joinTokens(this->tokens, returnOpen + 1, returnClose);

                        std::size_t nameIndex = returnClose + 1;
                        if (nameIndex >= this->tokens.size() || this->tokens[nameIndex].type != Token_t::IDENTIFIER) {
                            throw std::runtime_error("Malformed function declaration: missing function name.");
                        }

                        std::string name = this->tokens[nameIndex].literal;
                        std::size_t paramsOpen = nameIndex + 1;
                        if (paramsOpen >= this->tokens.size() || this->tokens[paramsOpen].literal != "(") {
                            throw std::runtime_error("Malformed function declaration: missing parameter list.");
                        }

                        std::size_t paramsClose = findMatching(this->tokens, paramsOpen, "(", ")");
                        std::vector<std::string> parameters = this->parseParameters(paramsOpen + 1, paramsClose);

                        std::size_t bodyOpen = paramsClose + 1;
                        if (bodyOpen >= this->tokens.size() || this->tokens[bodyOpen].literal != "{") {
                            throw std::runtime_error("Malformed function declaration: missing function body.");
                        }

                        std::size_t bodyClose = findMatching(this->tokens, bodyOpen, "{", "}");
                        std::vector<Token> body(this->tokens.begin() + static_cast<std::ptrdiff_t>(bodyOpen + 1), this->tokens.begin() + static_cast<std::ptrdiff_t>(bodyClose));

                        std::string header = joinTokens(this->tokens, i, bodyOpen);
                        this->functions.emplace(name, FunctionDef{.name = name, .returnType = returnType, .header = std::move(header), .parameters = std::move(parameters), .body = std::move(body)});
                        i = bodyClose + 1;
                        continue;
                    }

                    ++i;
                }
            }

            std::vector<std::vector<Token>> splitStatements(const std::vector<Token>& body) const {
                std::vector<std::vector<Token>> statements;
                std::vector<Token> current;
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;

                for (const auto& token : body) {
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "{") {
                        ++brace;
                    } else if (token.literal == "}") {
                        if (brace > 0) {
                            --brace;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    }

                    if (token.literal == ";" && paren == 0 && brace == 0 && bracket == 0) {
                        if (!current.empty()) {
                            statements.push_back(current);
                            current.clear();
                        }
                        continue;
                    }

                    current.push_back(token);
                }

                if (!current.empty()) {
                    statements.push_back(current);
                }

                return statements;
            }

            std::string tokensToText(const std::vector<Token>& items) const {
                std::string text;
                for (std::size_t i = 0; i < items.size(); ++i) {
                    const std::string& literal = items[i].literal;
                    if (i > 0 && literal != ")" && literal != "," && literal != ";" && literal != "." && literal != "::" && literal != ">" && items[i - 1].literal != "(" && items[i - 1].literal != "." && items[i - 1].literal != "::" && items[i - 1].literal != "<") {
                        text += ' ';
                    }
                    text += literal;
                }
                return text;
            }

            std::string statementToAsm(const std::vector<Token>& statement) const {
                if (statement.empty()) {
                    return {};
                }

                AttachedBlock attached = splitAttachedBlock(statement);
                if (attached.hasBlock && !attached.header.empty()) {
                    std::size_t probe = 0;
                    std::string name = this->readNamePath(attached.header, probe, attached.header.size());
                    if (name == "std::while") {
                        return "WHILE " + this->tokensToText(attached.header) + " { ... }";
                    }
                    if (name == "std::for") {
                        return "FOR " + this->tokensToText(attached.header) + " { ... }";
                    }
                    if (name == "std::forrange") {
                        return "FORRANGE " + this->tokensToText(attached.header) + " { ... }";
                    }
                }

                if (statement.front().literal == "return") {
                    if (statement.size() == 1) {
                        return "RETURN";
                    }
                    std::vector<Token> expr(statement.begin() + 1, statement.end());
                    return "RETURN " + this->tokensToText(expr);
                }

                if (statement.size() == 2 && statement[0].type == Token_t::IDENTIFIER && statement[1].type == Token_t::OPERATOR && (statement[1].literal == "++" || statement[1].literal == "--")) {
                    return std::string(statement[1].literal == "++" ? "INC " : "DEC ") + statement[0].literal;
                }

                std::size_t assignment = findTopLevelAssignment(statement);
                if (assignment != statement.size()) {
                    std::vector<Token> lhs(statement.begin(), statement.begin() + static_cast<std::ptrdiff_t>(assignment));
                    std::vector<Token> rhs(statement.begin() + static_cast<std::ptrdiff_t>(assignment + 1), statement.end());

                    if (isDeclarationLike(lhs)) {
                        return "DECLARE " + declarationType(lhs) + " " + declarationName(lhs) + (rhs.empty() ? "" : " = " + this->tokensToText(rhs));
                    }

                    return "MOVE " + this->tokensToText(lhs) + ", " + this->tokensToText(rhs);
                }

                if (isDeclarationLike(statement)) {
                    return "DECLARE " + declarationType(statement) + " " + declarationName(statement);
                }

                std::size_t callOpen = statement.size();
                for (std::size_t i = 0; i < statement.size(); ++i) {
                    if (statement[i].literal == "(") {
                        callOpen = i;
                        break;
                    }
                }

                if (callOpen != statement.size()) {
                    std::vector<Token> callHead(statement.begin(), statement.begin() + static_cast<std::ptrdiff_t>(callOpen));
                    return "CALL " + this->tokensToText(callHead) + this->tokensToText(std::vector<Token>(statement.begin() + static_cast<std::ptrdiff_t>(callOpen), statement.end()));
                }

                return "EVAL " + this->tokensToText(statement);
            }

            ExecResult executeLoopStatement(const std::string& name, const std::vector<Token>& header, const std::vector<Token>& body, Scope& scope) {
                if (name == "std::while") {
                    std::size_t callOpen = 0;
                    while (callOpen < header.size() && header[callOpen].literal != "(") {
                        ++callOpen;
                    }

                    if (callOpen >= header.size()) {
                        throw std::runtime_error("std::while is missing its condition.");
                    }

                    std::size_t callClose = findMatching(header, callOpen, "(", ")");
                    std::vector<std::pair<std::size_t, std::size_t>> slices = splitArgumentSlices(header, callOpen + 1, callClose);
                    if (slices.size() != 1) {
                        throw std::runtime_error("std::while expects one condition argument.");
                    }

                    std::vector<Token> conditionTokens(header.begin() + static_cast<std::ptrdiff_t>(slices[0].first), header.begin() + static_cast<std::ptrdiff_t>(slices[0].second));
                    while (valueTruthy(this->evaluateExpression(conditionTokens, scope))) {
                        ExecResult result = this->executeBody(body, scope);
                        if (result.returned) {
                            return result;
                        }
                    }

                    return {};
                }

                if (name == "std::for") {
                    std::size_t callOpen = 0;
                    while (callOpen < header.size() && header[callOpen].literal != "(") {
                        ++callOpen;
                    }

                    if (callOpen >= header.size()) {
                        throw std::runtime_error("std::for is missing its header.");
                    }

                    std::size_t callClose = findMatching(header, callOpen, "(", ")");
                    std::size_t first = findTopLevelDelimited(header, callOpen + 1, callClose, ";");
                    std::size_t second = findTopLevelDelimited(header, first + 1, callClose, ";");
                    if (first == callClose || second == callClose) {
                        throw std::runtime_error("std::for expects init; condition; step.");
                    }

                    std::vector<Token> initTokens(header.begin() + static_cast<std::ptrdiff_t>(callOpen + 1), header.begin() + static_cast<std::ptrdiff_t>(first));
                    std::vector<Token> conditionTokens(header.begin() + static_cast<std::ptrdiff_t>(first + 1), header.begin() + static_cast<std::ptrdiff_t>(second));
                    std::vector<Token> stepTokens(header.begin() + static_cast<std::ptrdiff_t>(second + 1), header.begin() + static_cast<std::ptrdiff_t>(callClose));

                    if (!initTokens.empty()) {
                        ExecResult initResult = this->executeStatement(initTokens, scope);
                        if (initResult.returned) {
                            return initResult;
                        }
                    }

                    while (conditionTokens.empty() || valueTruthy(this->evaluateExpression(conditionTokens, scope))) {
                        ExecResult result = this->executeBody(body, scope);
                        if (result.returned) {
                            return result;
                        }

                        if (!stepTokens.empty()) {
                            ExecResult stepResult = this->executeStatement(stepTokens, scope);
                            if (stepResult.returned) {
                                return stepResult;
                            }
                        }
                    }

                    return {};
                }

                if (name == "std::forrange") {
                    std::size_t callOpen = 0;
                    while (callOpen < header.size() && header[callOpen].literal != "(") {
                        ++callOpen;
                    }

                    if (callOpen >= header.size()) {
                        throw std::runtime_error("std::forrange is missing its header.");
                    }

                    std::size_t callClose = findMatching(header, callOpen, "(", ")");
                    std::vector<std::pair<std::size_t, std::size_t>> slices = splitArgumentSlices(header, callOpen + 1, callClose);
                    if (slices.size() < 2) {
                        throw std::runtime_error("std::forrange expects start and end arguments.");
                    }

                    std::vector<Token> startTokens(header.begin() + static_cast<std::ptrdiff_t>(slices[0].first), header.begin() + static_cast<std::ptrdiff_t>(slices[0].second));
                    std::vector<Token> endTokens(header.begin() + static_cast<std::ptrdiff_t>(slices[1].first), header.begin() + static_cast<std::ptrdiff_t>(slices[1].second));
                    std::vector<Token> stepTokens;
                    if (slices.size() >= 3) {
                        stepTokens.assign(header.begin() + static_cast<std::ptrdiff_t>(slices[2].first), header.begin() + static_cast<std::ptrdiff_t>(slices[2].second));
                    }

                    std::int64_t startValue = valueToInt(this->evaluateExpression(startTokens, scope));
                    std::int64_t endValue = valueToInt(this->evaluateExpression(endTokens, scope));
                    std::int64_t stepValue = stepTokens.empty() ? 1 : valueToInt(this->evaluateExpression(stepTokens, scope));

                    if (stepValue == 0) {
                        throw std::runtime_error("std::forrange step cannot be zero.");
                    }

                    scope.types["it"] = "std::int64";
                    for (std::int64_t index = startValue; stepValue > 0 ? index < endValue : index > endValue; index += stepValue) {
                        scope.values["it"] = index;
                        ExecResult result = this->executeBody(body, scope);
                        if (result.returned) {
                            return result;
                        }
                    }

                    return {};
                }

                throw std::runtime_error("Unsupported loop construct: " + name);
            }

            void dumpClasses(std::ostream& out) const {
                for (std::size_t i = 0; i < this->tokens.size();) {
                    if (this->tokens[i].literal == "class" && i + 1 < this->tokens.size() && this->tokens[i + 1].type == Token_t::IDENTIFIER) {
                        const std::string className = this->tokens[i + 1].literal;
                        std::size_t bodyOpen = i + 2;
                        while (bodyOpen < this->tokens.size() && this->tokens[bodyOpen].literal != "{") {
                            ++bodyOpen;
                        }
                        if (bodyOpen >= this->tokens.size()) {
                            break;
                        }

                        std::size_t bodyClose = findMatching(this->tokens, bodyOpen, "{", "}");
                        out << "STRUCT " << className << '\n';

                        std::vector<Token> fieldStatement;
                        std::size_t paren = 0;
                        std::size_t brace = 0;
                        std::size_t bracket = 0;
                        for (std::size_t j = bodyOpen + 1; j < bodyClose; ++j) {
                            const auto& token = this->tokens[j];
                            if (token.literal == "(") {
                                ++paren;
                            } else if (token.literal == ")") {
                                if (paren > 0) {
                                    --paren;
                                }
                            } else if (token.literal == "{") {
                                ++brace;
                            } else if (token.literal == "}") {
                                if (brace > 0) {
                                    --brace;
                                }
                            } else if (token.literal == "[") {
                                ++bracket;
                            } else if (token.literal == "]") {
                                if (bracket > 0) {
                                    --bracket;
                                }
                            }

                            if (token.literal == ";" && paren == 0 && brace == 0 && bracket == 0) {
                                if (!fieldStatement.empty()) {
                                    if (isDeclarationLike(fieldStatement)) {
                                        out << "  DECLARE " << declarationType(fieldStatement) << " " << declarationName(fieldStatement) << '\n';
                                    }
                                    fieldStatement.clear();
                                }
                                continue;
                            }

                            fieldStatement.push_back(token);
                        }

                        out << "ENDSTRUCT" << '\n' << '\n';
                        i = bodyClose + 1;
                        continue;
                    }

                    ++i;
                }
            }

            std::vector<std::string> parseParameters(std::size_t start, std::size_t end) const {
                std::vector<std::string> parameters;
                std::size_t segmentStart = start;
                std::size_t paren = 0;
                std::size_t bracket = 0;
                std::size_t templateDepth = 0;

                auto pushParameter = [&](std::size_t segmentEnd) {
                    if (segmentEnd <= segmentStart) {
                        return;
                    }

                    std::size_t lastIdentifier = segmentEnd;
                    for (std::size_t i = segmentEnd; i > segmentStart; --i) {
                        if (this->tokens[i - 1].type == Token_t::IDENTIFIER) {
                            lastIdentifier = i - 1;
                            break;
                        }
                    }

                    if (lastIdentifier < segmentEnd) {
                        parameters.push_back(this->tokens[lastIdentifier].literal);
                    }
                };

                for (std::size_t i = start; i < end; ++i) {
                    const auto& token = this->tokens[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    } else if (token.literal == "<") {
                        ++templateDepth;
                    } else if (token.literal == ">") {
                        if (templateDepth > 0) {
                            --templateDepth;
                        }
                    }

                    if (token.literal == "," && paren == 0 && bracket == 0 && templateDepth == 0) {
                        pushParameter(i);
                        segmentStart = i + 1;
                    }
                }

                pushParameter(end);
                return parameters;
            }

            Value invoke(const std::string& name, const std::vector<Value>& args) {
                if (name == "std::println") {
                    for (std::size_t i = 0; i < args.size(); ++i) {
                        if (i > 0) {
                            std::cout << ' ';
                        }
                        std::cout << valueToString(args[i]);
                    }
                    std::cout << std::endl;
                    return std::monostate{};
                }

                if (name == "if") {
                    if (args.size() < 3) {
                        throw std::runtime_error("if requires three arguments.");
                    }
                    return valueTruthy(args[0]) ? args[1] : args[2];
                }

                if (name == "std::while" || name == "std::for" || name == "std::forrange") {
                    throw std::runtime_error(name + " requires an attached block body.");
                }

                auto functionIt = this->functions.find(name);
                if (functionIt == this->functions.end()) {
                    throw std::runtime_error("Unknown function: " + name);
                }

                const FunctionDef& function = functionIt->second;
                Scope scope;
                for (std::size_t i = 0; i < function.parameters.size(); ++i) {
                    Value argument = i < args.size() ? args[i] : std::monostate{};
                    scope.values[function.parameters[i]] = std::move(argument);
                }

                ExecResult result = this->executeBody(function.body, scope);
                if (result.returned) {
                    return result.value;
                }

                return defaultValueForType(function.returnType);
            }

            std::vector<std::pair<std::size_t, std::size_t>> splitArgumentSlices(const std::vector<Token>& items, std::size_t start, std::size_t end) {
                std::vector<std::pair<std::size_t, std::size_t>> slices;
                std::size_t segmentStart = start;
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;

                for (std::size_t i = start; i < end; ++i) {
                    const auto& token = items[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "{") {
                        ++brace;
                    } else if (token.literal == "}") {
                        if (brace > 0) {
                            --brace;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    }

                    if (token.literal == "," && paren == 0 && brace == 0 && bracket == 0) {
                        slices.emplace_back(segmentStart, i);
                        segmentStart = i + 1;
                    }
                }

                if (segmentStart <= end) {
                    slices.emplace_back(segmentStart, end);
                }

                return slices;
            }

            ExecResult executeBody(const std::vector<Token>& body, Scope& scope) {
                std::vector<Token> statement;
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;

                for (std::size_t i = 0; i < body.size(); ++i) {
                    const auto& token = body[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "{") {
                        ++brace;
                    } else if (token.literal == "}") {
                        if (brace > 0) {
                            --brace;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    }

                    if (token.literal == ";" && paren == 0 && brace == 0 && bracket == 0) {
                        if (!statement.empty()) {
                            ExecResult result = this->executeStatement(statement, scope);
                            if (result.returned) {
                                return result;
                            }
                            statement.clear();
                        }
                        continue;
                    }

                    statement.push_back(token);

                    if (token.literal == "}" && paren == 0 && brace == 0 && bracket == 0) {
                        AttachedBlock attached = splitAttachedBlock(statement);
                        if (attached.hasBlock) {
                            ExecResult result = this->executeStatement(statement, scope);
                            if (result.returned) {
                                return result;
                            }
                            statement.clear();
                        }
                    }
                }

                if (!statement.empty()) {
                    ExecResult result = this->executeStatement(statement, scope);
                    if (result.returned) {
                        return result;
                    }
                }

                return {};
            }

            ExecResult executeStatement(std::vector<Token> statement, Scope& scope) {
                if (this->conf.isDebug) {
                    std::cout << "EXEC: " << this->tokensToText(statement) << '\n';
                }

                while (!statement.empty() && statement.front().literal == "<") {
                    std::size_t close = findMatching(statement, 0, "<", ">");
                    statement.erase(statement.begin(), statement.begin() + static_cast<std::ptrdiff_t>(close + 1));
                }

                if (statement.empty()) {
                    return {};
                }

                if (statement.front().literal == "return") {
                    if (statement.size() == 1) {
                        return ExecResult{.returned = true, .value = std::monostate{}};
                    }

                    std::vector<Token> expr(statement.begin() + 1, statement.end());
                    Value value = this->evaluateExpression(expr, scope);
                    return ExecResult{.returned = true, .value = value};
                }

                if (statement.size() == 2 && statement[0].type == Token_t::IDENTIFIER && statement[1].type == Token_t::OPERATOR && (statement[1].literal == "++" || statement[1].literal == "--")) {
                    std::string name = statement[0].literal;
                    Value& target = this->requireVariable(name, scope);
                    std::int64_t current = valueToInt(target);
                    current += (statement[1].literal == "++") ? 1 : -1;
                    target = current;
                    if (this->conf.isDebug) {
                        std::cout << "  -> " << name << " = " << current << '\n';
                    }
                    return {};
                }

                AttachedBlock attached = splitAttachedBlock(statement);
                if (attached.hasBlock && !attached.header.empty()) {
                    std::size_t probe = 0;
                    std::string name = this->readNamePath(attached.header, probe, attached.header.size());
                    if (name == "std::while" || name == "std::for" || name == "std::forrange") {
                        return this->executeLoopStatement(name, attached.header, attached.body, scope);
                    }
                }

                std::size_t assignment = findTopLevelAssignment(statement);
                if (assignment != statement.size()) {
                    std::vector<Token> lhs(statement.begin(), statement.begin() + static_cast<std::ptrdiff_t>(assignment));
                    std::vector<Token> rhs(statement.begin() + static_cast<std::ptrdiff_t>(assignment + 1), statement.end());
                    Value value = this->evaluateExpression(rhs, scope);

                    if (isDeclarationLike(lhs)) {
                        std::string name = declarationName(lhs);
                        std::string typeName = declarationType(lhs);
                        scope.types[name] = typeName;
                        scope.values[name] = coerceToType(typeName, std::move(value));
                        if (this->conf.isDebug) {
                            std::cout << "  -> declare " << name << " = " << valueToString(scope.values[name]) << '\n';
                        }
                        return {};
                    }

                    this->assignTarget(lhs, std::move(value), scope);
                    return {};
                }

                if (isDeclarationLike(statement)) {
                    std::string name = declarationName(statement);
                    std::string typeName = declarationType(statement);
                    scope.types[name] = typeName;
                    scope.values[name] = defaultValueForType(typeName);
                    if (this->conf.isDebug) {
                        std::cout << "  -> declare " << name << " = " << valueToString(scope.values[name]) << '\n';
                    }
                    return {};
                }

                (void)this->evaluateExpression(statement, scope);
                return {};
            }

            void assignTarget(const std::vector<Token>& lhs, Value value, Scope& scope) {
                if (lhs.empty()) {
                    throw std::runtime_error("Empty assignment target.");
                }

                if (lhs.size() == 1 && lhs[0].type == Token_t::IDENTIFIER) {
                    std::string name = lhs[0].literal;
                    auto typeIt = scope.types.find(name);
                    if (typeIt != scope.types.end()) {
                        scope.values[name] = coerceToType(typeIt->second, std::move(value));
                    } else {
                        scope.values[name] = std::move(value);
                    }
                    return;
                }

                std::size_t dot = std::string::npos;
                for (std::size_t i = 0; i < lhs.size(); ++i) {
                    if (lhs[i].literal == ".") {
                        dot = i;
                        break;
                    }
                }

                if (dot == std::string::npos || dot == 0 || dot + 1 >= lhs.size()) {
                    throw std::runtime_error("Unsupported assignment target.");
                }

                std::string baseName = lhs[0].literal;
                Value& baseValue = this->requireVariable(baseName, scope);
                if (!std::holds_alternative<ObjectPtr>(baseValue) || std::get<ObjectPtr>(baseValue) == nullptr) {
                    throw std::runtime_error("Assignment target is not an object: " + baseName);
                }

                ObjectPtr object = std::get<ObjectPtr>(baseValue);
                std::string fieldName = lhs[dot + 1].literal;
                object->fields[fieldName] = std::move(value);
            }

            Value& requireVariable(const std::string& name, Scope& scope) {
                auto valueIt = scope.values.find(name);
                if (valueIt == scope.values.end()) {
                    throw std::runtime_error("Unknown variable: " + name);
                }
                return valueIt->second;
            }

            Value evaluateExpression(const std::vector<Token>& items, Scope& scope) {
                std::size_t pos = 0;
                return this->parseExpression(items, pos, items.size(), scope, 1);
            }

            Value parseExpression(const std::vector<Token>& items, std::size_t& pos, std::size_t end, Scope& scope, int minPrecedence) {
                Value left = this->parseUnary(items, pos, end, scope);

                while (pos < end) {
                    const Token& token = items[pos];
                    if (token.type != Token_t::OPERATOR || token.literal == "=") {
                        break;
                    }

                    auto precedenceIt = Dyno::Compiler::opPrecedence.find(token.literal);
                    if (precedenceIt == Dyno::Compiler::opPrecedence.end()) {
                        break;
                    }

                    int precedence = static_cast<int>(precedenceIt->second);
                    if (precedence < minPrecedence) {
                        break;
                    }

                    std::string op = token.literal;
                    ++pos;
                    int nextMinPrecedence = Dyno::Compiler::isRightAssociative.contains(op) ? precedence : precedence + 1;
                    Value right = this->parseExpression(items, pos, end, scope, nextMinPrecedence);
                    left = this->applyBinary(op, std::move(left), std::move(right));
                }

                return left;
            }

            Value parseUnary(const std::vector<Token>& items, std::size_t& pos, std::size_t end, Scope& scope) {
                if (pos < end && items[pos].type == Token_t::OPERATOR) {
                    const std::string& op = items[pos].literal;
                    if (op == "+" || op == "-" || op == "!") {
                        ++pos;
                        Value inner = this->parseUnary(items, pos, end, scope);
                        if (op == "+") {
                            return inner;
                        }
                        if (op == "-") {
                            if (std::holds_alternative<double>(inner)) {
                                return -std::get<double>(inner);
                            }
                            return -valueToInt(inner);
                        }
                        return !valueTruthy(inner);
                    }
                }

                return this->parsePrimary(items, pos, end, scope);
            }

            std::string readNamePath(const std::vector<Token>& items, std::size_t& pos, std::size_t end) const {
                if (pos >= end || items[pos].type != Token_t::IDENTIFIER) {
                    throw std::runtime_error("Expected identifier.");
                }

                std::string name = items[pos].literal;
                ++pos;

                while (pos + 1 <= end && pos < end && (items[pos].literal == "::" || items[pos].literal == ".")) {
                    std::string separator = items[pos].literal;
                    if (pos + 1 >= end || items[pos + 1].type != Token_t::IDENTIFIER) {
                        break;
                    }
                    name += separator + items[pos + 1].literal;
                    pos += 2;
                }

                return name;
            }

            Value parsePrimary(const std::vector<Token>& items, std::size_t& pos, std::size_t end, Scope& scope) {
                if (pos >= end) {
                    return std::monostate{};
                }

                const Token& token = items[pos];
                if (token.literal == "(") {
                    ++pos;
                    Value inner = this->parseExpression(items, pos, end, scope, 1);
                    if (pos >= end || items[pos].literal != ")") {
                        throw std::runtime_error("Unclosed parenthesized expression.");
                    }
                    ++pos;
                    return inner;
                }

                if (token.type == Token_t::LITERAL) {
                    ++pos;
                    const std::string& literal = token.literal;
                    if (!literal.empty() && (literal.front() == '"' || literal.front() == '\'')) {
                        return stripQuotes(literal);
                    }

                    if (literal.find('.') != std::string::npos || literal.find('e') != std::string::npos || literal.find('E') != std::string::npos) {
                        return std::stod(literal);
                    }

                    return static_cast<std::int64_t>(std::stoll(literal));
                }

                if (token.type == Token_t::IDENTIFIER) {
                    std::string name = this->readNamePath(items, pos, end);

                    if (pos < end && items[pos].literal == "<" && (name == "if" || name == "Function")) {
                        skipTemplateArguments(items, pos);
                    }

                    if (pos < end && items[pos].literal == "(") {
                        std::size_t callOpen = pos;
                        std::size_t callClose = findMatching(items, callOpen, "(", ")");
                        std::vector<std::pair<std::size_t, std::size_t>> argSlices = this->splitArgumentSlices(items, callOpen + 1, callClose);

                        if (name == "if") {
                            if (argSlices.size() < 3) {
                                throw std::runtime_error("if requires three arguments.");
                            }

                            Value condition = this->evaluateExpression(std::vector<Token>(items.begin() + static_cast<std::ptrdiff_t>(argSlices[0].first), items.begin() + static_cast<std::ptrdiff_t>(argSlices[0].second)), scope);
                            std::size_t branchIndex = valueTruthy(condition) ? 1 : 2;
                            Value chosen = this->evaluateExpression(std::vector<Token>(items.begin() + static_cast<std::ptrdiff_t>(argSlices[branchIndex].first), items.begin() + static_cast<std::ptrdiff_t>(argSlices[branchIndex].second)), scope);
                            pos = callClose + 1;
                            return chosen;
                        }

                        std::vector<Value> args;
                        args.reserve(argSlices.size());
                        for (const auto& slice : argSlices) {
                            std::vector<Token> argument(items.begin() + static_cast<std::ptrdiff_t>(slice.first), items.begin() + static_cast<std::ptrdiff_t>(slice.second));
                            if (!argument.empty()) {
                                args.push_back(this->evaluateExpression(argument, scope));
                            }
                        }
                        pos = callClose + 1;
                        return this->invoke(name, args);
                    }

                    if (name.find('.') != std::string::npos) {
                        std::vector<std::string> parts;
                        std::size_t start = 0;
                        while (start < name.size()) {
                            std::size_t dot = name.find('.', start);
                            if (dot == std::string::npos) {
                                parts.push_back(name.substr(start));
                                break;
                            }
                            parts.push_back(name.substr(start, dot - start));
                            start = dot + 1;
                        }

                        Value current = this->requireVariable(parts.front(), scope);
                        for (std::size_t i = 1; i < parts.size(); ++i) {
                            if (!std::holds_alternative<ObjectPtr>(current) || std::get<ObjectPtr>(current) == nullptr) {
                                throw std::runtime_error("Field access on non-object: " + parts.front());
                            }

                            ObjectPtr object = std::get<ObjectPtr>(current);
                            auto fieldIt = object->fields.find(parts[i]);
                            if (fieldIt == object->fields.end()) {
                                current = std::monostate{};
                            } else {
                                current = fieldIt->second;
                            }
                        }
                        return current;
                    }

                    auto variableIt = scope.values.find(name);
                    if (variableIt != scope.values.end()) {
                        return variableIt->second;
                    }

                    if (name == "true") {
                        return true;
                    }
                    if (name == "false") {
                        return false;
                    }
                    if (name == "nullptr") {
                        return std::monostate{};
                    }

                    return std::monostate{};
                }

                throw std::runtime_error("Unsupported expression token: " + token.literal);
            }

            std::vector<Value> evaluateArgumentList(const std::vector<Token>& items, std::size_t start, std::size_t end, Scope& scope) {
                std::vector<Value> values;
                std::vector<Token> current;
                std::size_t paren = 0;
                std::size_t brace = 0;
                std::size_t bracket = 0;
                std::size_t templateDepth = 0;

                auto pushCurrent = [&]() {
                    if (!current.empty()) {
                        values.push_back(this->evaluateExpression(current, scope));
                        current.clear();
                    }
                };

                for (std::size_t i = start; i < end; ++i) {
                    const auto& token = items[i];
                    if (token.literal == "(") {
                        ++paren;
                    } else if (token.literal == ")") {
                        if (paren > 0) {
                            --paren;
                        }
                    } else if (token.literal == "{") {
                        ++brace;
                    } else if (token.literal == "}") {
                        if (brace > 0) {
                            --brace;
                        }
                    } else if (token.literal == "[") {
                        ++bracket;
                    } else if (token.literal == "]") {
                        if (bracket > 0) {
                            --bracket;
                        }
                    } else if (token.literal == "<") {
                        ++templateDepth;
                    } else if (token.literal == ">") {
                        if (templateDepth > 0) {
                            --templateDepth;
                        }
                    }

                    if (token.literal == "," && paren == 0 && brace == 0 && bracket == 0 && templateDepth == 0) {
                        pushCurrent();
                        continue;
                    }

                    current.push_back(token);
                }

                pushCurrent();
                return values;
            }

            Value applyBinary(const std::string& op, Value left, Value right) {
                if (op == "+") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) + valueToDouble(right);
                    }
                    if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
                        return valueToString(left) + valueToString(right);
                    }
                    return valueToInt(left) + valueToInt(right);
                }
                if (op == "-") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) - valueToDouble(right);
                    }
                    return valueToInt(left) - valueToInt(right);
                }
                if (op == "*") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) * valueToDouble(right);
                    }
                    return valueToInt(left) * valueToInt(right);
                }
                if (op == "/") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) / valueToDouble(right);
                    }
                    return valueToInt(left) / valueToInt(right);
                }
                if (op == "%") {
                    return valueToInt(left) % valueToInt(right);
                }
                if (op == "==") {
                    return valueToString(left) == valueToString(right);
                }
                if (op == "!=") {
                    return valueToString(left) != valueToString(right);
                }
                if (op == "<") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) < valueToDouble(right);
                    }
                    return valueToInt(left) < valueToInt(right);
                }
                if (op == "<=") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) <= valueToDouble(right);
                    }
                    return valueToInt(left) <= valueToInt(right);
                }
                if (op == ">") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) > valueToDouble(right);
                    }
                    return valueToInt(left) > valueToInt(right);
                }
                if (op == ">=") {
                    if (std::holds_alternative<double>(left) || std::holds_alternative<double>(right)) {
                        return valueToDouble(left) >= valueToDouble(right);
                    }
                    return valueToInt(left) >= valueToInt(right);
                }
                if (op == "&&") {
                    return valueTruthy(left) && valueTruthy(right);
                }
                if (op == "||") {
                    return valueTruthy(left) || valueTruthy(right);
                }

                throw std::runtime_error("Unsupported binary operator: " + op);
            }
    };
} // namespace Dyno::Runtime