// Operator.h
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace Dyno::Compiler {
    enum class OperatorType : std::uint8_t {
        Uniary = 0x00,
        Binary = 0x01,
        Unknown = 0x67
    };

    static std::unordered_map<std::string, OperatorType> operators = {
        {"__add__", OperatorType::Binary}, // +
        {"__sub__", OperatorType::Binary}, // -
        {"__mul__", OperatorType::Binary}, // *
        {"__div__", OperatorType::Binary}, // /
        {"__mod__", OperatorType::Binary}, // %
        {"__assign__", OperatorType::Binary}, // =
        {"__eq__", OperatorType::Binary}, // ==
        {"__neq__", OperatorType::Binary}, // !=
        {"__lt__", OperatorType::Binary}, // <
        {"__gt__", OperatorType::Binary}, // >
        {"__leq__", OperatorType::Binary}, // <=
        {"__geq__", OperatorType::Binary}, // >=
        {"__and__", OperatorType::Binary}, // &&
        {"__or__", OperatorType::Binary}, // ||
        {"__xor__", OperatorType::Binary}, // ^
        {"__not__", OperatorType::Uniary}, // !
        {"__index__", OperatorType::Binary}, // []
        {"__call__", OperatorType::Binary}, // ()
        {"__inc__", OperatorType::Uniary}, // ++
        {"__dec__", OperatorType::Uniary}, // --
        {"__add_assign__", OperatorType::Binary}, // +=
        {"__sub_assign__", OperatorType::Binary}, // -=
        {"__mul_assign__", OperatorType::Binary}, // *=
        {"__div_assign__", OperatorType::Binary}, // /=
        {"__mod_assign__", OperatorType::Binary}, // %=
        {"__bit_and__", OperatorType::Binary}, // &
        {"__bit_or__", OperatorType::Binary}, // |
        {"__bit_not__", OperatorType::Uniary}, // ~
        {"__lshift__", OperatorType::Binary}, // <<
        {"__rshift__", OperatorType::Binary}  // >>
    };

    static std::unordered_map<std::string, std::string> symToText = {
        {"+", "__add__"},
        {"-", "__sub__"},
        {"*", "__mul__"},
        {"/", "__div__"},
        {"%", "__mod__"},
        {"=", "__assign__"},
        {"==", "__eq__"},
        {"!=", "__neq__"},
        {"<", "__lt__"},
        {">", "__gt__"},
        {"<=", "__leq__"},
        {">=", "__geq__"},
        {"&&", "__and__"},
        {"||", "__or__"},
        {"^", "__xor__"},
        {"!", "__not__"},
        {"[]", "__index__"},
        {"()", "__call__"},
        {"++", "__inc__"},
        {"--", "__dec__"},
        {"+=", "__add_assign__"},
        {"-=", "__sub_assign__"},
        {"*=", "__mul_assign__"},
        {"/=", "__div_assign__"},
        {"%=", "__mod_assign__"},
        {"&", "__bit_and__"},
        {"|", "__bit_or__"},
        {"~", "__bit_not__"},
        {"<<", "__lshift__"},
        {">>", "__rshift__"},
    };

    static const std::unordered_set<char> opSym {
        '+', '-', '*', '/', '%', '=', '!', '<', '>', '&', '|', '^'
    };

    static const std::unordered_set<char> punctuationSym {
        '(', ')', '{', '}', '[', ']', ';', ',', '.', ':', '"', '\'', '\\', '?',
    };

    static const std::unordered_set<std::string> delimiter {
        "(", ")", "{", "}", "[", "]", ";", ",", ".", ":", "\"", "'", "\\", "?", "::", "->"
    };

    static const std::unordered_map<std::string, std::uint8_t> opPrecedence = {
        {"=", 1}, {"+=", 1}, {"-=", 1}, {"*=", 1}, {"/=", 1}, {"%=", 1},
        {"||", 2},
        {"&&", 3},
        {"|", 4},
        {"^", 5},
        {"&", 6},
        {"==", 7}, {"!=", 7},
        {"<", 8}, {">", 8}, {"<=", 8}, {">=", 8},
        {"+", 9}, {"-", 9},
        {"*", 10}, {"/", 10}, {"%", 10},
        {"!", 11}, // Unary operators have higher precedence
        {"++", 11}, {"--", 11},
        {"~", 11},
        {"<<", 12}, {">>", 12},
    };

    static const std::unordered_set<std::string> isRightAssociative = std::unordered_set<std::string> {
        "=", "+=", "-=", "*=", "/=", "%="
    };
}