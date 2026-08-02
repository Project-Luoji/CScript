// Object.h
#pragma once

#include <cstdint>
#include <vector>

#include "bin.h"

namespace Dyno::Type {
    
    class Instruction {

        public:
            virtual ~Instruction() = default;

            Instruction() : _start(0), length(0) {} // Null Instruction
            Instruction(std::uint64_t start, std::uint64_t length) : _start(start), length(length) {}

            std::uint64_t start() const noexcept { return _start; }
            std::uint64_t size() const noexcept { return length; }

        private:
            std::uint64_t _start;
            std::uint64_t length;

    };
} // namespace Dyno::Type