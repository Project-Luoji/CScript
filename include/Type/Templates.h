// Templates.h
#pragma once

#include <concepts>
#include <vector>
#include <string>
#include <memory>

#include "Type.h"
#include "Primative.h"

namespace Dyno::Type {
    class Template : public Type {
        public:
            explicit Template(const std::string& name) : Type(name, {}, {}) {}

            std::uint64_t isTemplate() const noexcept override{
                return this->templates.size();
            }

            // A template should NOT have a root
            std::shared_ptr<const Type> getRoot() const noexcept override {
                return nullptr;
            }

            std::shared_ptr<const Type> getDerivedOf(const std::uint64_t depth) const noexcept override {
                return nullptr;
            }

            std::shared_ptr<const Type> parent() const noexcept override {
                return nullptr;
            }
            
            
        private:
            std::vector<std::shared_ptr<const Objective>> templates;
    };
} // Dyno::Type