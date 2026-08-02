// Runtime.h
#pragma once

#include <type_traits>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <concepts>

#include "../Util/Exception.h"
#include "../Util/Configure.h"

#include "../Type/Type.h"
#include "../Type/Object.h"
#include "../Type/Primative.h"

namespace Dyno::Runtime {

    using namespace Dyno::Type;
    using namespace Dyno::Util;
    using namespace Dyno::Exception;

    using CSType = Dyno::Type::Type;

    class Runtime {
        public:
            Runtime(const Configure& conf) : conf(conf), _stack({}) {}

            template<typename R>
            requires std::is_integral_v<R>
            std::uint64_t write(R val) noexcept {
                std::uint64_t size = sizeof(R); 
                std::uint64_t addr = this->_stack.size();
    
                this->_stack.reserve(addr + size); // Optimazation;

                for(std::uint64_t i = 0; i < size; ++i) {
                    this->_stack.push_back(static_cast<std::uint8_t>((val >> (i * 8)) & 0xFF));
                }

                return addr;
            }

            template<typename R>
            requires std::is_integral_v<R>
            R read(std::uint64_t addr) const {
                std::uint64_t size = sizeof(R);
                if(addr + size > this->_stack.size()) {
                    throwException(this->conf.exceptSet[0x04], "Dyno::Runtime::read: Address out of range");
                }
                R val = 0;
                for(std::uint64_t i = 0; i < size; ++i) {
                    val |= (static_cast<R>(this->_stack[addr + i]) << (i * 8));
                }
                return val;
            }

            std::vector<std::vector<std::uint8_t>> read(const std::uint64_t& addr, const std::shared_ptr<const CSType>& type) noexcept {
                std::vector<std::vector<std::uint8_t>> result = {};
                std::uint64_t start{0uz};
                result.reserve(type->fieldSize());
                
                if(addr > this->_stack.size()) {
                    throwException(this->conf.exceptSet[0x04], "Dyno::Runtime::read: Address out of range");
                } else {
                    start = addr;
                }
                
                for(const std::uint64_t& offset : type->getOffsets()) {
                    std::uint64_t end = start + offset;
                    if(end > this->_stack.size()) {
                        throwException(this->conf.exceptSet[0x04], "Dyno::Runtime::read: Address out of range");
                    }
                    std::vector<std::uint8_t> field(this->_stack.begin() + start, this->_stack.begin() + end);
                    result.push_back(field);
                }
                
                return result;
            }

        private:
            std::vector<std::uint8_t> _stack;
            Configure conf;
    };
    

} // namespace Dyno::Runtime