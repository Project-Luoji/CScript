// Statement.h
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "Tokens.h"

namespace Dyno::Compiler {

    enum class Statement_t : std::uint8_t {
        DECLARATION = 0x0,
        FUNCTION_CALL = 0x1,
        COMPLEX = 0x2,
        UNKNOWN = 0xFF
    };

    enum class Block_t : std::uint8_t {
        Class = 0x0,
        Function = 0x1,
        Declaration = 0x2,
        Unknown = 0xFF
    };

    struct Statement {
        std::vector<Token> tokens;
        Statement_t type = Statement_t::UNKNOWN;
    };
    
    struct Block  {
      Statement initial;
      std::vector<Statement> statements;
      std::vector<Block> inner;
      Block_t type = Block_t::Unknown;
    };
} // Dyno::Compiler