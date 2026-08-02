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
        // 1. 解析函数句柄（args[0]）
        if (args.empty()) {
            throw std::runtime_error("CALL: missing function handle");
        }
    
        uint64_t funcHandle = 0;
        for (size_t i = 0; i < args[0].first.size() && i < 8; ++i) {
            funcHandle |= static_cast<uint64_t>(args[0].first[i]) << (i * 8);
        }
    
        // 2. 解析返回地址信息（args 最后两个）
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
    
        // 3. 传递参数：将 args[1..n-2] 写入寄存器或帧
        //    这里简化，假设参数通过寄存器传递（最多 16 个）
        //    实际实现中，你需要根据类型写入环境
    
        // 4. 调用函数
        //    注意：这里调用的是 Env::call，它内部负责切换上下文
        env.call(funcHandle);
    
        // 注意：call 返回后，执行权已经切换到被调用函数
        // 当前指令流不会继续执行
    };
    
    ASMFunction Jump = [](Env& env, const Args& args) {
        if (args.empty()) {
            throw std::runtime_error("JUMP: missing target");
        }
    
        // 解析目标地址
        uint64_t target = 0;
        for (size_t i = 0; i < args[0].first.size() && i < 8; ++i) {
            target |= static_cast<uint64_t>(args[0].first[i]) << (i * 8);
        }
    
        // 特殊约定：target == 0 表示返回调用者（从栈中恢复状态）
        // 但 Env 接口没有暴露“返回”操作，所以需要在 Env 实现中处理
        // 这里我们直接让 JUMP 修改 IP，但 Env 没提供 IP 接口
    
        // 问题：Env 接口没有暴露指令指针（IP）的设置方法。
        // 因此，JUMP 的实现需要依赖 Env 的内部状态。
        // 如果你的 Env 实现提供了类似 setIP() 的方法，就在这里调用。
    };
} // namespace Dyno::Functional