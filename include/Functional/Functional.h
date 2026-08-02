// Functional.h
// This will the be will the File declaring the how Functions are stored.
// A function should be read as 
// A VECTOR OF {
//  ASM ARG...
// }
// 
// // How Can I write them permanently to a size?
// MEMORY STORATION:
// || ======= ||
// || MOVE    || 0x1 0x2 ||
// || CALL    || 0x3 0x4 0x5 0x6 ||
// || JUMP    || 0x7 0x8 ||
// || ====== || 0x9 ||
// Obviously, I can do {1, x} // as 1 is for the asm code
// but x....  

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <functional>

#include "../Type/Primative.h"
#include "../Type/Type.h"
#include "Assembly.h"

namespace Dyno::Type {
    class Functional_t final : public Objective {
        public:

            Functional_t(const std::vector<std::shared_ptr<const Objective>> types) : Objective("Functional", {8}, ptr::get(bin::get())), target(types) {}

            std::uint64_t isTemplate() const noexcept override {
                return target.size(); // We can have multiple template parameters for functions, e.g., return type and argument types.
            }

        private:
            std::vector<std::shared_ptr<const Objective>> target;
    };
}

namespace Dyno::Functional {
    struct Function {
        std::string name;
        std::uint64_t frameSize = 0;
        std::vector<std::string> parameters;
        std::vector<Command> body;
        std::shared_ptr<const Dyno::Type::Type> returnType;
        std::function<void(Dyno::Runtime::Env&)> entry;

        void run(Dyno::Runtime::Env& env) const {
            if (this->entry) {
                this->entry(env);
            }
        }
    };
};