// Strhelp.h
#pragma once

#include <string>
#include <cctype>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "../Compiler/Operator.h"

namespace Dyno::Util {
    using namespace Dyno::Compiler;

    bool isStringDelimiter(char c) noexcept {
        return c == '"' || c == '\'';
    }
    
    bool isOperatorStart(char c) noexcept {
        return opSym.find(c) != opSym.end();
    }

    bool isPunctuation(char c) noexcept {
        return punctuationSym.find(c) != punctuationSym.end();
    }

    bool isIdentifier(char c) noexcept {
        return std::isalnum(c) || c == '_';
    }

    void trim(std::string& str){
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), str.end());
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        
        for (char c : str) {
            if (c == delimiter) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string unescape(const std::string& str) {
        std::string result = str;
        for(std::size_t i = 0; i < result.size(); ++i) {
            if(result[i] == '\\' && i + 1 < result.size()) {
                char nextChar = result[i + 1];
                switch(nextChar) {
                    case 'n': result.replace(i, 2, "\n"); break;
                    case 't': result.replace(i, 2, "\t"); break;
                    case 'r': result.replace(i, 2, "\r"); break;
                    case '\\': result.replace(i, 2, "\\"); break;
                    case '"': result.replace(i, 2, "\""); break;
                    case '\'': result.replace(i, 2, "\'"); break;
                    default: break; // Leave unknown escape sequences as is
                }
            }
        }
        return result;
    }

} // namespace Dyno::Util