// Assembly.h
#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include <utility>
#include <memory>

#include "../Type/Type.h"


namespace Dyno::Functional {

    // Final, idealistic, efficient assembly illustration for CScript.
    /** Custom Type
     * struct Human {
     *   std::str 0x1;
     *   std::str 0x2;
     *   std::uint32_t 0x3;
     *   std::uint64_t 0x4;
     * };
     * 
     * DECLARE Function <std::str> 0x67(std::ptr<Human>& 0x0 = nullptr, std::ptr<std::str> 0x1 = 0x0, ){
     *      DECLARE std::str 0x2;
     *      DECLARE ptr<std::str> 0x4;
     *      CALL .__get__(0x4, __get__(0x0, 0x1));
     *      READ 0x4 0x2;
     *      MOVE 0x2 0x1;
     * };
     */

    
    enum class Assembly: std::uint8_t {
        CALL = 0x00,
        JUMP = 0x02,
        WRITE = 0x03,
        READ = 0x04,
        MOVE = 0x05,
        DECLARE = 0x06,
        STRUCT = 0x07
    };


    enum class Operand : std::uint8_t {
        NONE = 0x00, // No operand, nullptr
        LITERAL = 0x01, // Literal value, e.g., int, float, string
        REFERENCE = 0x02, // Reference to a variable
        POINTER = 0x03, // Pointer to a variable
        COPY = 0x04, // Copy of a variable
        Functional = 0x05 // Function call
    };
    
    struct Argument {
        Operand type = Operand::NONE;
        std::vector<std::pair<std::vector<std::uint8_t>, std::shared_ptr<const Dyno::Type::Type>>> values; // Pair of value and its type
    };
    
    struct Command {
        Assembly command;
        std::vector<Argument> arguments;
    };

    
}