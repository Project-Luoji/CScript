// Keyword.h
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace Dyno::Compiler {
    static const std::unordered_set<std::string> keywords = {
        "include", "namespace", "class", "typedef"
    };

    static const std::unordered_map<std::string, std::uint8_t> predefined = {
      {"true", 0x1},
      {"false", 0x0},
      {"nullptr", 0x0}
    };
} // namespace Dyno::Compiler