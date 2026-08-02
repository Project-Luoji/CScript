// ASMFunction.h
#pragma once

#include "../Runtime/Env.h"
#include "../Type/Type.h"

namespace Dyno::Functional {
    using Env = Dyno::Runtime::Env;
    using CASMFunction = void(*)(Dyno::Runtime::Env&, const std::vector<std::pair<std::vector<std::uint8_t>, std::shared_ptr<const Type::Type>>>&); // :Nerd: for the C-riders
    using ASMFunction = std::function<void(Dyno::Runtime::Env&,const std::vector<std::pair<std::vector<std::uint8_t>, std::shared_ptr<const Type::Type>>>&)>;
    using Args = std::vector<std::pair<std::vector<std::uint8_t>, std::shared_ptr<const Type::Type>>>;
    
    ASMFunction Call = [](Env& env, const Args& args) {
        if (args.empty()) {
            throw std::runtime_error("CALL: missing function handle");
        }
    
        uint64_t funcHandle = 0;
        for (size_t i = 0; i < args[0].first.size() && i < 8; ++i) {
            funcHandle |= static_cast<uint64_t>(args[0].first[i]) << (i * 8);
        }
    
        if (args.size() < 3) {
            throw std::runtime_error("CALL: missing return address");
        }
    
        uint64_t retOffset = 0;
        for (size_t i = 0; i < args[args.size() - 2].first.size() && i < 8; ++i) {
            retOffset |= static_cast<uint64_t>(args[args.size() - 2].first[i]) << (i * 8);
        }
    
        uint64_t retLen = 0;
        for (size_t i = 0; i < args[args.size() - 1].first.size() && i < 8; ++i) {
            retLen |= static_cast<uint64_t>(args[args.size() - 1].first[i]) << (i * 8);
        }
        env.call(funcHandle);
    };
    
    ASMFunction Jump = [](Env& env, const Args& args) {
        if (args.empty()) {
            throw std::runtime_error("JUMP: missing target");
        }
        
        uint64_t target = 0;
        for (size_t i = 0; i < args[0].first.size() && i < 8; ++i) {
            target |= static_cast<uint64_t>(args[0].first[i]) << (i * 8);
        }
    };
} // namespace Dyno::Functional