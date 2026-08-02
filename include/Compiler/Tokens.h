// Tokens.h
#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Dyno::Compiler {
    enum class Token_t : std::uint8_t {
        UNKNOWN = 0x00,
        IDENTIFIER = 0x01,
        KEYWORD = 0x02,
        LITERAL = 0x03,
        OPERATOR = 0x04,
        SEPARATOR = 0x05,
        COMMENT = 0x06,
        END,
        END_OF_FILE = 0x08
    };

   struct Token {
       Token_t type = Token_t::UNKNOWN;
       std::string literal;
       std::uint64_t line;
       std::uint64_t start;
       std::uint64_t end;
   };

   struct TokenStream {
       std::vector<Token> tokens;
       std::uint64_t Index;
       std::string source;
   };
} // namespace Dyno::Compiler