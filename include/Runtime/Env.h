// Env.h
#pragma once

#include <cstdint>
#include <concepts>
#include <vector>
#include <memory>

#include "../Type/Type.h"
#include "../Type/Object.h"
#include "../Util/Configure.h"
#include "../Util/Exception.h"
#include "../Functional/Assembly.h"
#include "../Functional/Functional.h"


namespace Dyno::Runtime {
    using Type = Dyno::Type::Type;
    using namespace Type;
    using namespace Functional;
    
    class Env {
        public:
            Env() = default;
            virtual ~Env() = default;

            /**
             * Wait for functional implementation.
             */
            virtual Function find(const std::uint64_t& handle) const noexcept = 0;
            
            virtual void call(const std::uint64_t& handle) noexcept {
                Function func = this->find(handle);

                this->callstack.push_back(this->line + 1);

                std::uint64_t funcLine = this->_stack.size();
                
                this->callstack.push_back(funcLine);
                this->_stack.resize(funcLine + func.frameSize); 
                func.run(*this);
                
            }

            /**
             * @brief Raw non-standard method of writing data. This is mainly used for writing raw data.
             * @param addr The address to write to.
             * @param _instance The data to write.
             * @return The address of the written data.
             */
            virtual std::uint64_t write(const std::uint64_t& addr, const std::vector<std::uint8_t>& _instance) noexcept = 0;

            /**
             * @brief overwrites a given length's data starting (inclusive) at a given address
             * @param addr The address to start overwriting at.
             * @param object The data to overwrite with.
             * @param type The type of the data to overwrite.
             * @return The address of the overwritten data.
             */
            virtual void overwrite(const std::uint64_t& addr, const std::vector<std::uint8_t>& object, const std::shared_ptr<const Type>& type) noexcept = 0;

            /**
             * @brief Standard method of obtaining data from the stack, given a address and a type.
             * @param addr The address to read from.
             * @param type The type of the data to read.
             * @return A Object containing the data read from the stack.
             */
            virtual std::vector<std::vector<std::uint8_t>> read(const std::uint64_t& addr, const std::shared_ptr<const Type>& type) const noexcept = 0;

            /**
             * @brief Non-standard method of reading. This method is used to read raw data only.
             * @param addr The address to read from.
             * @param length The length of the data to read.
             * @return A vector of uint64_t containing the data read from the stack.
             */
            virtual std::vector<std::uint64_t> read(const std::uint64_t& addr, const std::uint64_t& length) const noexcept = 0;

            virtual std::uint64_t getStackSize() const noexcept = 0;
            virtual std::uint64_t getHeapSize() const noexcept = 0;
            virtual std::vector<Exception::Exception> getExceptions() const noexcept{
                return this->conf.exceptSet;
            }
            
        protected:
            Util::Configure conf;
            std::vector<std::uint8_t> _stack;
            std::uint64_t line;
            const std::vector<Functional::Command>* current = nullptr;
            std::vector<std::uint64_t> callstack;
    };
} // namespace Dyno::Runtime