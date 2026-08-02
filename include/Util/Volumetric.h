// Volumetric.h
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <stdexcept>
#include <utility>
#include <memory>
#include <sstream>
#include <type_traits>
#include <concepts>
#include <cmath>
#include <typeinfo>

namespace Dyno::Util {

    template <typename T>
    requires std::integral<T>
    struct Child {
        std::unique_ptr<T[]> instance;
        std::vector<std::pair<std::uint64_t, std::uint64_t>> freed; // addr, length
    };

    struct Address {
        std::uint64_t addr;
        std::uint64_t offset;
    };

    template <typename T>
    requires std::integral<T> 
    class Volumetric {
        public:
            
            virtual ~Volumetric() = default;
            Volumetric() noexcept: _volume(0), fixed(256), parent() {}

            Volumetric(const Volumetric&) = delete;
            Volumetric(std::uint64_t _init = 4) noexcept: parent(), _volume(_init), fixed(256){
                for(std::uint64_t i = 0; i < std::ceil(_init/static_cast<double>(this->fixed)); ++i) {
                    this->parent.emplace_back(std::make_unique<Child<T>>(Child<T>{std::make_unique<T[]>(this->fixed), this->fixed, {0, this->fixed}}));
                }
            }

            std::uint64_t volume() const noexcept {
                return this->_volume;
            }

            Address find(std::uint64_t addr) const {
                if(addr >= this->_volume) {
                    throw std::out_of_range("Volumetric::find: address out of range");
                }

                if (this->parent.empty()) {
                    throw std::runtime_error("Volumetric::find: no instances available");
                }

                
                for(std::uint64_t i = 0; i < this->parent.size(); ++i) {
                    const auto& instance = this->parent[i];
                    if (addr < instance->length) {
                        return Address{i, instance->length - addr};
                    }
                    addr -= instance->length;
                }

                throw std::invalid_argument("Volumetric::find: address not found");
            }

            std::uint64_t allocate(std::uint64_t length) {
                if (length <= 0) {
                    throw std::invalid_argument("Volumetric::allocate: length must be greater than 0\nUse Volumetric::deallocate to free memory.");
                }

                std::uint64_t addr = this->_volume;
                
                for(std::uint64_t i = 0; i < std::ceil(length/static_cast<double>(this->fixed)); ++i) {
                    this->parent.emplace_back(std::make_unique<Child<T>>(Child<T>{std::make_unique<T[]>(this->fixed), this->fixed, {0, this->fixed}}));
                    this->_volume += this->fixed;
                }
                return addr;
            }

            void deallocate(std::uint64_t addr, std::uint64_t length) {
                if (length <= 0) {
                    throw std::invalid_argument("Volumetric::deallocate: length must be greater than 0\nUse Volumetric::allocate to allocate memory.");
                }

                if (addr + length > this->_volume) {
                    throw std::out_of_range("Volumetric::deallocate: address out of range");
                }       
            }

            std::uint64_t append(const T& _instance) {
                Child<T>& back = this->parent.back();

                if (back.freed.empty()) {
                    std::uint64_t addr = this->allocate(this->fixed);
                    back.instance = std::make_unique<T[]>(this->fixed);
                    back.instance[0] = _instance;
                    back.freed.emplace_back(addr + 1, this->fixed);
                    return addr;
                }

                for (auto& instance : back.freed) {
                    if(instance.second > 0) {
                        std::uint64_t addr = instance.first;
                        back.instance[addr] = _instance;
                        instance.first += 1;
                        instance.second -= 1;
                        return addr;
                    }
                }
            } 

        
            


        private:
            std::vector<std::unique_ptr<Child<T>>> parent;
            std::uint64_t _volume;
            std::uint64_t fixed; // 256 is the default fixed size cannot be changed after initialization
    };
} // namespace Dyno::Util