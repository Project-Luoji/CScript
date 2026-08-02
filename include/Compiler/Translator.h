// Translator.h
// Used to translate variables and other elements into static codes.

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../Functional/Assembly.h"
#include "../Type/Primative.h"
#include "../Util/Configure.h"
#include "../Util/Exception.h"
#include "../Util/Strhelp.h"

#include "Tokens.h"
#include "Statement.h"

namespace Dyno::Compiler {

    class Translator {
        public:
            Translator(const Util::Configure& conf, std::vector<Token> tokens) noexcept : conf(conf), tokens(tokens) {}

            std::vector<Block> block() const noexcept {
                std::vector<Block> blocks = {};
                std::vector<Statement> statements = this->group();
                std::uint64_t start = 0;

                while (start < statements.size()) {
                    if (statements[start].tokens.empty()) {
                        ++start;
                        continue;
                    }

                    if (statements[start].tokens.size() == 1 && statements[start].tokens[0].literal == "}") {
                        ++start;
                        continue;
                    }

                    Block current = this->parseBlock(statements, start);
                    blocks.push_back(current);
                    ++start;
                }

                return blocks;
            }

            std::vector<Statement> group() const noexcept {
                std::vector<Statement> statements;
                std::vector<Token> currentStatement;

                for (const auto& token : this->tokens) {
                    currentStatement.push_back(token);

                    if (token.literal == ";" || token.literal == "{" || token.literal == "}") {
                        statements.push_back(Statement{currentStatement, Statement_t::UNKNOWN});
                        currentStatement.clear();
                    }
                }

                if (!currentStatement.empty()) {
                    statements.push_back(Statement{currentStatement, Statement_t::UNKNOWN});
                }

                for (auto& statement : statements) {
                    if (statement.tokens.empty()) {
                        Exception::throwException(this->conf.exceptSet[0x2], "Compiler::Translator::Group: Empty statement found during translation.");
                        continue;
                    }

                    for (std::uint64_t i{0uz}; i < statement.tokens.size(); ++i) {
                        Token current = statement.tokens[i];

                        if (current.type == Token_t::UNKNOWN) {
                            Exception::throwException(this->conf.exceptSet[0x2], "Compiler::Translator::Group: Unknown token found during translation. Token: " + current.literal + " at line: " + std::to_string(current.line));
                        }

                        if (current.type == Token_t::IDENTIFIER) {
                            Token next = (i + 1 < statement.tokens.size()) ? statement.tokens[i + 1] : Token{ .type = Token_t::END_OF_FILE, .literal = "EOF", .line = current.line, .start = current.start, .end = current.start };
                            ++i;

                            if (next.type == Token_t::END_OF_FILE) {
                                Exception::throwException(this->conf.exceptSet[0x2], "Compiler::Translator::Group: Unexpected end of statement after identifier. Token: " + current.literal + " at line: " + std::to_string(current.line));
                            }

                            if (next.type == Token_t::SEPARATOR) {
                                if (next.literal == "(") {
                                    statement.type = Statement_t::FUNCTION_CALL;
                                } else {
                                    statement.type = Statement_t::DECLARATION;
                                }
                            }

                            if (next.type == Token_t::OPERATOR) {
                                statement.type = Statement_t::FUNCTION_CALL;
                                continue;
                            }

                            if (next.type == Token_t::IDENTIFIER) {
                                statement.type = Statement_t::DECLARATION;
                                continue;
                            }

                            if (next.type == Token_t::SEPARATOR && next.literal == "{") {
                                statement.type = Statement_t::COMPLEX;
                                continue;
                            }

                            continue;
                        }

                        if (current.type == Token_t::SEPARATOR) {
                            if (current.literal == "{") {
                                statement.type = Statement_t::COMPLEX;
                            } else if (current.literal == ";") {
                                statement.type = Statement_t::DECLARATION;
                            }
                        }
                    }
                }

                return statements;
            }

        private:
            std::vector<Token> tokens;
            Util::Configure conf;

            static std::vector<std::uint8_t> encodeBytes(const std::string& literal) noexcept {
                return std::vector<std::uint8_t>(literal.begin(), literal.end());
            }

            static Dyno::Functional::Argument makeArgument(const Token& token) noexcept {
                Dyno::Functional::Argument argument;

                if (token.type == Token_t::IDENTIFIER) {
                    argument.type = Dyno::Functional::Operand::REFERENCE;
                } else if (token.type == Token_t::LITERAL || token.type == Token_t::KEYWORD) {
                    argument.type = Dyno::Functional::Operand::LITERAL;
                } else if (token.type == Token_t::OPERATOR) {
                    argument.type = Dyno::Functional::Operand::COPY;
                } else {
                    argument.type = Dyno::Functional::Operand::NONE;
                }

                argument.values.push_back({encodeBytes(token.literal), Dyno::Type::bin::get()});
                return argument;
            }

            static Block_t classifyBlock(const Statement& statement) noexcept {
                if (statement.tokens.empty()) {
                    return Block_t::Unknown;
                }

                if (statement.tokens.front().literal == "class") {
                    return Block_t::Class;
                }

                if (statement.type == Statement_t::DECLARATION) {
                    return Block_t::Declaration;
                }

                for (const auto& token : statement.tokens) {
                    if (token.literal == "{") {
                        return Block_t::Function;
                    }
                }

                return Block_t::Unknown;
            }

            Block parseBlock(const std::vector<Statement>& statements, std::uint64_t& start) const noexcept {
                Block block;
                block.initial = statements[start];

                if (block.initial.tokens.empty()) {
                    Exception::throwException(this->conf.exceptSet[0x2], "Compiler::Translator::parseBlock: Empty initial statement found during block parsing.");
                }

                block.type = classifyBlock(block.initial);

                bool hasOpenBrace = false;
                for (const auto& token : block.initial.tokens) {
                    if (token.literal == "{") {
                        hasOpenBrace = true;
                        break;
                    }
                }

                if (!hasOpenBrace) {
                    return block;
                }

                std::vector<Statement> innerStatements;
                std::vector<Block> innerBlocks;

                for (std::uint64_t i = start + 1; i < statements.size(); ++i) {
                    const Statement& stmt = statements[i];

                    if (stmt.tokens.empty()) {
                        Exception::throwException(this->conf.exceptSet[0x2], "Compiler::Translator::parseBlock: Empty statement found during block parsing.");
                    }

                    // Handle closing brace
                    if (stmt.tokens.size() == 1 && stmt.tokens[0].literal == "}") {
                        start = i;
                        continue;
                    }

                    bool opensBlock = false;
                    for (const auto& token : stmt.tokens) {
                        if (token.literal == "{") {
                            opensBlock = true;
                            break;
                        }
                    }

                    // Recurse into nested blocks that begin on this statement.
                    if (opensBlock) {
                        Block innerBlock = parseBlock(statements, i);
                        innerBlocks.push_back(innerBlock);
                        continue;
                    }

                    innerStatements.push_back(stmt);
                }

                block.statements = std::move(innerStatements);
                block.inner = std::move(innerBlocks);
                return block;
            }
    };
} // Dyno::Compiler