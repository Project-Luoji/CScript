// Exception.h
#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <filesystem>

#include "Yaml.h"

namespace Dyno::Util{

};

namespace Dyno::Exception {

    enum class ExceptionType : std::uint8_t {
        NOEXCEPTION = 0x00,
        NULL_POINTER = 0x01,
        INVALID_ARGUMENT = 0x02,
        COMPILE_ERROR = 0x03,
        RUNTIME_ERROR = 0x04,
        BAD_ALLOC = 0x05,
        UNKNOWN_ERROR = 0x06,
        EXCEPTION = 0x07
    };

    static std::unordered_map<ExceptionType, std::string> exceptions = {
        {ExceptionType::NOEXCEPTION, "No Exception"},
        {ExceptionType::NULL_POINTER, "Null Pointer Exception"},
        {ExceptionType::INVALID_ARGUMENT, "Invalid Argument Exception"},
        {ExceptionType::COMPILE_ERROR, "Compile Error Exception"},
        {ExceptionType::RUNTIME_ERROR, "Runtime Error Exception"},
        {ExceptionType::BAD_ALLOC, "Bad Allocation Exception"},
        {ExceptionType::UNKNOWN_ERROR, "Unknown Error Exception"},
        {ExceptionType::EXCEPTION, "General Exception"}
    };

    static std::unordered_map<std::string, ExceptionType> exceptionTypes = {
        {"NOEXCEPTION", ExceptionType::NOEXCEPTION},
        {"NULL_POINTER", ExceptionType::NULL_POINTER},
        {"INVALID_ARGUMENT", ExceptionType::INVALID_ARGUMENT},
        {"COMPILE_ERROR", ExceptionType::COMPILE_ERROR},
        {"RUNTIME_ERROR", ExceptionType::RUNTIME_ERROR},
        {"BAD_ALLOC", ExceptionType::BAD_ALLOC},
        {"UNKNOWN_ERROR", ExceptionType::UNKNOWN_ERROR},
        {"EXCEPTION", ExceptionType::EXCEPTION}
    };

    struct Exception {
        std::string type;
        std::string message;
    };

    std::vector<Exception> getExceptSet(const std::string& file = "include/Util/Predefined/builtin.yaml", const std::vector<std::string>& args = {}){
        if(file.empty()) {
            std::cerr << "Dyno::Exception::getExceptSet: file path is empty.\n";
            std::exit(EXIT_FAILURE);
        }

        if(!std::filesystem::exists(file)) {
            std::cerr << "Dyno::Exception::getExceptSet: file does not exist: " << file << "\n";
            std::exit(EXIT_FAILURE);
        }

        std::ifstream f(file);
        if(!f.is_open() || !f.good()) {
            std::cerr << "Dyno::Exception::getExceptSet: failed to open file: " << file << "\n";
            std::exit(EXIT_FAILURE);
        }

        if(f.peek() == std::ifstream::traits_type::eof()) {
            std::cerr << "Dyno::Exception::getExceptSet: file is empty: " << file<< "\n";
            std::exit(EXIT_FAILURE);
        }

        std::string line;
        std::string currentType;
        std::vector<Exception> exceptions;

        while(std::getline(f, line)) {
            if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments

            // Language has no space
            if(line.ends_with(":") || !line.starts_with(" ")) {
                size_t pos = line.find(':');
                line = line.substr(0, pos);
                
            }
        }

        return exceptions;
    }

    void throwException(const Exception& e, const std::string& message) noexcept {
        std::cout << e.message << "\n" << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}