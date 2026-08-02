// Lexer.h
#pragma once

#include <cctype>
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string_view>

#include "Tokens.h"


#include "../Util/Configure.h"
#include "../Util/Exception.h"
#include "../Util/Strhelp.h"

namespace Dyno::Compiler {
    using namespace Dyno::Util;
    
    class Lexer {
        public:
            explicit Lexer() = default;

            Lexer(const std::string& source, const Configure& conf) noexcept : conf(conf){
                std::filesystem::path path{source};
                std::ifstream file(path);
                if(!file.is_open()) {
                    Exception::throwException(this->conf.exceptSet[0x1], "Failed to open file: " + source);
                }
                this->source = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            }
            
            virtual ~Lexer() = default;

            void tokensize() {
                this->tokens.clear();

                this->tokens.reserve(1024);
                Token temp = this->nextToken();

                while(temp.type != Token_t::END_OF_FILE) {
                    this->tokens.push_back(temp);
                    if(temp.type == Token_t::END_OF_FILE) {
                        break;
                    }
                    temp = this->nextToken();
                }

                if(temp.type == Token_t::END_OF_FILE) {
                    this->tokens.push_back(temp);
                }

                if(this->conf.isDebug) {
                    std::cout << "Finalizing token parsing. Total tokens: " << this->tokens.size() << std::endl;
                }

                for(std::uint64_t i{0uz}; i < this->tokens.size(); ++i) {
                    Token current = this->tokens[i];
                    if(current.type == Token_t::UNKNOWN) {
                        Exception::throwException(this->conf.exceptSet[0x6], "Unknown token encountered at line: " + std::to_string(current.line) + ", start: " + std::to_string(current.start) + ", Literal: " + current.literal);
                    }
                }

                // Replace this with --ctx flag.
                if(this->conf.isDebug) {
                    std::cout << "Writing to file: " << std::filesystem::current_path() / "tokens.ctx" << std::endl;

                    if(!std::filesystem::exists(std::filesystem::current_path() / "tokens.ctx")) {
                        std::ofstream outFile(std::filesystem::current_path() / "tokens.ctx");
                        if(!outFile.is_open()) {
                            Exception::throwException(this->conf.exceptSet[0x1], "Failed to create file: " + (std::filesystem::current_path() / "tokens.ctx").string());
                        }

                        if(!outFile) {
                            Exception::throwException(this->conf.exceptSet[0x1], "Failed to write to file: " + (std::filesystem::current_path() / "tokens.ctx").string());
                        }

                        if(!outFile.good()) {
                            Exception::throwException(this->conf.exceptSet[0x1], "File stream is not good for writing: " + (std::filesystem::current_path() / "tokens.ctx").string());
                        }

                        if(!outFile.eof()) {
                            outFile.clear();
                        }
                    }
                    
                    this->writeToFile((std::filesystem::current_path() / "tokens.ctx").string());
                }
            }

            std::vector<Token> getTokens() const noexcept {
                return this->tokens;
            }

            void writeToFile(const std::string& filename) {
                std::ofstream outFile(filename);
                if(!outFile.is_open()) {
                    Exception::throwException(this->conf.exceptSet[0x1], "Failed to open file for writing: " + filename);
                }


                for(const auto& token : this->tokens) {
                    outFile << "Token: " << token.literal 
                            << ", Type: " << static_cast<int>(token.type) 
                            << ", Line: " << token.line 
                            << ", Start: " << token.start 
                            << ", End: " << token.end << "\n";
                }
            }
            
        private:
            std::vector<Token> tokens;
            std::uint64_t current = 0;
            std::uint64_t line {};
            std::uint64_t start{}, end {}; 
            std::string source;
            Configure conf;

        Token nextToken() {
            this->skipWhiteSpace();
            
            if( this->current > this->source.size()) {
                return Token{ .type = Token_t::END_OF_FILE, .literal = "EOF", .line = this->line, .start = this->start, .end = this->start };
            }

            char c = this->peek();

            if(c == '/') {
                char nextChar = (this->current + 1 < this->source.size()) ? this->source[this->current + 1] : '\0';
                if(nextChar == '/') {
                    // Single-line comment
                    while(this->peek() != '\n' && this->peek() != '\0') {
                        this->advance();
                    }
                    return this->nextToken(); // Skip the comment and get the next token
                } else if(nextChar == '*') {
                    // Multi-line comment
                    this->advance(); // Skip '/'
                    this->advance(); // Skip '*'
                    while(true) {
                        if(this->peek() == '*' && (this->current + 1 < this->source.size()) && this->source[this->current + 1] == '/') {
                            this->advance(); // Skip '*'
                            this->advance(); // Skip '/'
                            break;
                        }
                        if(this->peek() == '\0') {
                            Exception::throwException(this->conf.exceptSet[0x3], "Unterminated multi-line comment at line: " + std::to_string(this->line));
                        }
                        if(this->peek() == '\n') {
                            ++this->line;
                            this->start = 1;
                        }
                        this->advance();
                    }
                    return this->nextToken(); // Skip the comment and get the next token
                }
            }
            
            if(std::isdigit(c)) {
                return this->readNumeric();
            } 

            if(isStringDelimiter(c)) {
                return this->readLiteral();
            }
            
            if(std::isalpha(c) || c == '_') {
                return this->readIdentifier();
            }
            
            if(isOperatorStart(c)) {
                return this->readOperator();  
            } 

            if(isPunctuation(c)) {
                return this->readPunctuation();
            }

            
            return Token{
                .type = Token_t::END_OF_FILE,
                .literal = "EOF",
                .line = this->line,
                .start = this->start,
                .end = this->start
            };
        }

        char peek() const noexcept { return this->current < source.size() ? this->source[this->current] : '\0'; }
        char advance() { char c = this->peek(); if(c != '\0'){ ++this->current; ++this->start;}; return c; }
        
        void skipWhiteSpace(){ 
            while( std::isspace(this->peek())) {
                if(this->peek() == '\n') {
                    ++this->line;
                    this->start = 1;
                }

                this->advance();
            }
        }

        Token readNumeric() {
            std::uint64_t startPos = this->current;
            
            // Read the integer part
            while(std::isdigit(this->peek()) || this->peek() == '.') {
                this->advance();
            }

            if(this->peek() == 'e' || this->peek() == 'E') {
                this->advance();
                if(this->peek() == '+' || this->peek() == '-') {
                    this->advance();
                }
                while(std::isdigit(this->peek())) {
                    this->advance();
                }
            }

            std::string number = this->source.substr(startPos, this->current - startPos);
            return Token{ .type = Token_t::LITERAL, 
                          .literal = number,
                          .line = this->line,
                          .start = this->start,
                          .end = this->start + (this->current - startPos)
            };
        }

        Token readLiteral() {
            std::uint64_t startPos = this->current;
            char quoteType = this->peek(); // Either ' or " 
            this->advance(); // Skip the opening quote || waste it

            while(this->peek() != quoteType && this->peek() != '\0') {
                if(this->peek() == '\\') {
                    this->advance(); // Skip the escape character
                }
                this->advance();
            }

            if(this->peek() == '\0') {
                Exception::throwException(this->conf.exceptSet[0x4], "Unterminated string literal at line: " + std::to_string(this->line));
            }

            this->advance(); // Skip the closing quote

            std::string literal = this->source.substr(startPos, this->current - startPos);
            return Token{ .type = Token_t::LITERAL, 
                          .literal = literal,
                          .line = this->line,
                          .start = this->start,
                          .end = this->start + (this->current - startPos)
            };
        }

        Token readIdentifier() {
            std::uint64_t startPos = this->current;
            
            while(std::isalnum(this->peek()) || this->peek() == '_') {
                this->advance();
            }

            std::string identifier = this->source.substr(startPos, this->current - startPos);
            return Token{ .type = Token_t::IDENTIFIER, 
                          .literal = identifier,
                          .line = this->line,
                          .start = this->start,
                          .end = this->start + (this->current - startPos)
            };
        }

        Token readOperator() {
            std::uint64_t startPos = this->current;
            std::string op;
            
            while(isOperatorStart(this->peek())) {
                op += this->advance();
            }

            if(symToText.find(op) != symToText.end()) {
                return Token{ .type = Token_t::OPERATOR, 
                              .literal = op,
                              .line = this->line,
                              .start = this->start,
                              .end = this->start + (this->current - startPos)
                };
            } else {
                Exception::throwException(this->conf.exceptSet[0x5], "Unknown operator: " + op);
                return Token{ .type = Token_t::UNKNOWN, .literal = "", .line = this->line, .start = this->start, .end = this->start };
            }
        }

        Token readPunctuation() {
            char c = this->peek();
            std::string punct(1, c);
            this->advance();

            if(c == ':') {
                if(this->peek() == ':') {
                    this->advance();
                    punct += ':';
                }
            }

            return Token{ .type = Token_t::SEPARATOR, 
                          .literal = punct,
                          .line = this->line,
                          .start = this->start,
                          .end = this->start + 1
            };
        } 
    };
} // Dyno::Compiler