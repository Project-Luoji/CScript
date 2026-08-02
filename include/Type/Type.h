// Type.h

#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <utility>

#include "Object.h"


namespace Dyno::Type {

    class Type {
        public:
            virtual ~Type() = default;

            /**
             * @brief Constructs a Type object with the given name, offsets, and optional parent type.
             * @param name<std::string> name of the type
             * @param offsets<std::vector<std::uint64_t>> offsets of the type, like a normal int32 would be {0x4}, but a arr will be {0x8, 0x8} as a child of a ptr, and a ptr will be {0x8} as a child of a ptr. This is used to calculate the size of the type.
             * @param _parent<std::shared_ptr<const Type>> optional parent type
             */
            Type(const std::string& name, const std::vector<std::uint64_t>& offsets, std::shared_ptr<const Type> _parent = nullptr) : 
                _name(name), _parent(_parent) {

                    if(this->_parent != nullptr) {
                        this->offsets = this->_parent->getOffsets();
                    } else {
                        this->offsets = {};
                    }

                    this->offsets.reserve(this->offsets.size() + offsets.size());
                    this->offsets.insert(this->offsets.end(), offsets.begin(), offsets.end());
                    this->length = this->offsets.size();
                }


            virtual std::shared_ptr<const Type> getRoot() const noexcept {
                std::shared_ptr<const Type> curr = this->_parent;
                while (curr && curr->_parent) {
                    curr = curr->parent();
                }
                return curr;
            }   

            virtual std::shared_ptr<const Type> getDerivedOf(const std::uint64_t depth) const noexcept {
                std::shared_ptr<const Type> curr = this->_parent;
                std::uint64_t currentDepth = 0;
                for(;curr != nullptr && currentDepth < depth; curr = curr->_parent, ++currentDepth){
                    if(currentDepth == depth) return curr;
                }
                return nullptr;
            }
            
            
            std::string name() const noexcept { return this->_name; }
            std::uint64_t size() const noexcept { return this->length; }
            std::uint64_t fieldSize() const noexcept { return this->offsets.size(); }
            virtual std::shared_ptr<const Type> parent() const noexcept { return this->_parent; }
            
            virtual std::uint64_t isTemplate() const noexcept { return 0; }

            bool operator==(const Type& other) const noexcept {
                return this->_name == other._name && this->length == other.length && this->_parent == other._parent;
            }

            virtual std::vector<std::uint64_t> getOffsets() const noexcept {
                return this->offsets;
            }
            

        private:

            Type() noexcept = default;
            
            std::string _name;
            std::uint64_t length;
            std::shared_ptr<const Type> _parent;

        protected:
            std::vector<std::uint64_t> offsets;
    };  

} // namespace Dyno::Type