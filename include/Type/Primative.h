// Primative.h
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Object.h"
#include "Type.h"

namespace Dyno::Type {

    class Objective : public Type {
        public:
            explicit Objective(const std::string& name, const std::vector<std::uint64_t>& instructions, const std::shared_ptr<const Objective>& parent = {}) : Type(name, instructions, parent) {}
    };
    
    class bin : public Objective {
        public:
            explicit bin() noexcept: Objective("bin", {1})  {}

            static std::shared_ptr<const bin> get() noexcept{
                static std::shared_ptr<const bin> ptr = std::make_shared<const bin>(bin{});
                return ptr;
            }
    };

    class ptr : public Objective {
        public:
            explicit ptr(std::shared_ptr<const Objective> type) noexcept: Objective("ptr", {8}), target({type}) {}

            static std::shared_ptr<const ptr> get(const std::shared_ptr<const Objective>& type) noexcept {
                static std::unordered_map<std::shared_ptr<const Type>, std::shared_ptr<const ptr>> cache;
                if(!cache.contains(type)) {
                    cache[type] = std::make_shared<const ptr>(ptr(type));
                }
                return cache[type];
            }

            virtual std::uint64_t isTemplate() const noexcept {
                return target.size(); // We only have one template parameters. Expected one;
            }
            
        private:
            std::vector<std::shared_ptr<const Objective>> target;
    };

    class arr : public Objective{
        public:
            explicit arr(std::shared_ptr<const Objective> type) noexcept: Objective("arr", {8}, ptr::get(type)), target({type}) {}

            static std::shared_ptr<const arr> get(const std::shared_ptr<const Objective>& type) noexcept {
                static std::unordered_map<std::shared_ptr<const Type>, std::shared_ptr<const arr>> cache;
                if(!cache.contains(type)) {
                    cache[type] = std::make_shared<const arr>(arr(type));
                }
                return cache[type];
            }

            virtual std::uint64_t isTemplate() const noexcept {
                return target.size();
            }
            
        private:
            std::vector<std::shared_ptr<const Objective>> target;
    };

    class void_t : public Objective {
        public :
            explicit void_t() noexcept: Objective("void", {}) {}

            static std::shared_ptr<const void_t> get() noexcept {
                static std::shared_ptr<const void_t> instance = std::make_shared<const void_t>(void_t{});
                return instance;
            }
    };

    class int8 : public Objective {
        public:
            explicit int8() noexcept: Objective("int8", {}, bin::get())  {}

            static std::shared_ptr<const int8> get() noexcept{
                static std::shared_ptr<const int8> ptr = std::make_shared<const int8>(int8{});
                return ptr;
            }
    };

    class uint8 : public Objective {
        public:
            explicit uint8() noexcept: Objective("uint8", {}, bin::get())  {}

            static std::shared_ptr<const uint8> get() noexcept{
                static std::shared_ptr<const uint8> ptr = std::make_shared<const uint8>(uint8{});
                return ptr;
            }
    };
} // namespace Dyno::Type