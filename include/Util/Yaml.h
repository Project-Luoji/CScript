// Yaml.h

/**
 * Written by Dynosuars, 2026
 * Purpose: This file contains the necessary tools for parsing and handling YAML files
 * in the forms of a std::unordered_map<std::string, std::variant<std::string, std::string>>.
 * If you want a full-featured YAML parser, please use a library such as yaml-cpp or libyaml
 * 
 * 这并不是一个合格的C++ YAML解析器，它只是一个辅助工具，用于将YAML文件解析为std::unordered_map<std::string, std::string>的形式。
 */

// Basic data structures
#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <utility>

// Basic output control
#include <iostream>

// Basic file operations
#include <filesystem>
#include <fstream>

namespace Dyno::Util{

    struct YNode {
        std::string key;
        std::string val;
        std::uint64_t prior;
    };

    using yaml = std::vector<YNode>;

    bool isBlank(const std::string& str) noexcept {
        for(const auto& ch : str){
            if(!std::isspace(ch) || ch == '\0' || ch == '\n' || ch == '\r') return false;
        }
        return true;
    }

    std::string trim(const std::string& str) noexcept {
        std::size_t first = str.find_first_not_of(" \"\'\t\n\r");
        if(first == std::string::npos) return ""; // All whitespace
        std::size_t last = str.find_last_not_of(" \"\'\t\n\r");
        return str.substr(first, last - first + 1);
    }

    /**
     * @brief Parse a YAML file based on the path provided and returns a std::unordered_map<std::string, std::string> representation of the YAML file.
     * 
     * @param _path 
     * @return yaml 
     */
    yaml parseYaml(const std::string& _path) noexcept {
        yaml result;
        
        std::filesystem::path path(_path);

        if(!std::filesystem::exists(path) || std::filesystem::is_directory(path)){ 
            std::cerr << "Dyno::Util::parseYaml: File is virtual.\n^ Maybe try creating a file?\n^ Path: " << _path << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::ifstream file {_path, std::ios::in};

        if(!file.is_open() || !file.good()){
            std::cerr << "Dyno::Util::parseYaml: Failed to open file.\n^ Path: " << _path << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if(file.peek() == std::ifstream::traits_type::eof()) {
            std::cerr << "Dyno::Util::parseYaml: File is empty.\n^ Path: " << _path << std::endl;
            std::exit(EXIT_FAILURE);
        }


        // If the file has a BOM, we need to skip it. The BOM is 3 bytes: 0xEF, 0xBB, 0xBF
        if(file.peek() == 0xEF) {
            char bom[3];
            file.read(bom, 3);
        }


        std::string line;
        while(std::getline(file, line)){

            if(line.empty() || line[0] == '#' || isBlank(line) || line == "\r") continue; // Skip empty lines and comments


            std::size_t delimX = line.find(':');
            if(delimX == std::string::npos) {
                std::cerr << "Dyno::Util::parseYaml: Invalid line in YAML file.\n^ Line: " << static_cast<int>(line[0]) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            std::string key = trim(line.substr(0, delimX));
            // Check if key is empty. 
            if(key.empty()) {
                std::cerr << "Dyno::Util::parseYaml: Invalid key in YAML file.\n^ Line: " << line << std::endl;
                std::exit(EXIT_FAILURE);
            }

            std::string value = trim(line.substr(delimX + 1));
            
            if(value.empty()) value = "nullptr"; // If value is empty, set it to "null"

            std::uint64_t prior = 0;
            for(const auto& ch : line) {
                if(ch == ' ') prior++;
                else break;
            }

            result.push_back({key, value, static_cast<std::uint64_t>(prior / 2)}); // Divide by 2 to get the actual indentation level
        }

        return result;
    }
    
} // namespace Dyno::Util