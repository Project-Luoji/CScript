#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "include/Runtime/Interpreter.h"
#include "include/Util/Configure.h"


#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

int main(int argc, char* argv[]) {

    #if defined(_WIN32) || defined(_WIN64)
        // Set console output to UTF-8
        SetConsoleOutputCP(CP_UTF8);
    #endif

    std::vector<std::string> args;
    std::string sourcePath = "test.csxx";
    for (int i = 1; i < argc; ++i) {
        std::string current = argv[i];
        if (sourcePath == "test.csxx" && !current.starts_with("--")) {
            sourcePath = current;
            continue;
        }
        args.push_back(current);
    }

    Dyno::Util::Configure conf = Dyno::Util::makeConf(args);

    try {
        Dyno::Runtime::Interpreter interpreter(sourcePath, conf);
        if (conf.Compile) {
            interpreter.dumpCompiledAsm(std::cout);
            return 0;
        }
        interpreter.run();
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    return 0;

}
