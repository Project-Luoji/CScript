// Configure.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <filesystem>
#include <fstream>

#include <iostream>

#include "Yaml.h"
#include "Exception.h"
#include "Strhelp.h"

namespace Dyno::Util {

    struct Configure {
        std::string version;
        std::string language;
        std::string category;
        bool isStrict;
        bool isDebug;
        bool Compile;
        bool run;

        std::vector<Exception::Exception> exceptSet;
    };

    Configure makeConf(const std::vector<std::string>& args = {}, const std::string& file = "current") {

        Configure conf;


        std::vector<std::pair<std::string, std::string>> raw = {
            {"version", "1.0.0"},
            {"language", "English"},
            {"category", "Standard"},
            {"isStrict", "false"},
            {"isDebug", "false"},
            {"Compile", "false"},
            {"run", "true"},
        };

        for(const auto& [key, value] : raw) {
            if(key == "version") conf.version = value;
            else if(key == "language") conf.language = value;
            else if(key == "category") conf.category = value;
            else if(key == "isStrict") conf.isStrict = (value == "true");
            else if(key == "isDebug") conf.isDebug = (value == "true");
            else if(key == "Compile") conf.Compile = (value == "true");
            else if(key == "run") conf.run = (value == "true");
        }

        for(int i = 0; i < args.size(); ++i) {
            std::string current = args[i];
            
            if(current == "") {
                std::cerr << "Dyno::Util::makeConf: Empty argument provided.\n^ Argument index: " << i << std::endl;
                std::exit(EXIT_FAILURE);
            }

            // TODO:
            // Turn this into a hashmap of flags and functions
            if(current == "--strict") {
                conf.isStrict = true;
            } else if(current == "--debug") {
                conf.isDebug = true;
            } else if(current == "--compile") {
                conf.Compile = true;
            } else if(current == "--run") {
                conf.run = true;
            }

            if(i + 1 < args.size()) {
                std::string next = args[i + 1];
                if(next == "") {
                    std::cerr << "Dyno::Util::makeConf: Empty argument provided.\n^ Argument index: " << i + 1 << std::endl;
                    std::exit(EXIT_FAILURE);
                }

                if(current == "--language") {
                    conf.language = next;
                    ++i;
                } else if(current == "--category") {
                    conf.category = next;
                    ++i;
                } else if(current == "--debug") {
                    conf.isDebug = (next == "true");
                    ++i;
                } else if(current == "--strict") {
                    conf.isStrict = (next == "true");
                    ++i;
                } else if(current == "--compile") {
                    conf.Compile = (next == "true");
                    ++i;
                } else if(current == "--run") {
                    conf.run = (next == "true");
                    ++i;
                }
            }
        }


        std::filesystem::path _path = file;
        if(file == "current") {
            _path = std::filesystem::current_path() / "include/Util/Predefined/builtin.yaml";
        }

        // std::cout << "Dyno::Util::makeConf: Reading configuration from: " << _path.string() << std::endl;

        std::vector<Dyno::Util::YNode> Exceptions = parseYaml(_path.string());
        std::vector<Exception::Exception> exceptSet;

        for(std::uint64_t i = 0; i < Exceptions.size(); ++i) {
            const auto& [key, value, prior] = Exceptions[i];

            // std::cout << "Currently reading: " << key << ": " << value << " (Priority: " << prior << ")\n";

            if(key == conf.language && prior == 0) {

                for(std::uint64_t j = i + 1; j < Exceptions.size(); ++j) {
                    const auto& [subKey, subValue, subPrior] = Exceptions[j];

                    if(subPrior <= prior) break;

                    if(subPrior == 1 && subKey == conf.category) {
                        for(int k = j + 1; k < Exceptions.size(); ++k) {
                            const auto& [subSubKey, subSubValue, subSubPrior] = Exceptions[k];

                            if(subSubPrior <= subPrior) break;

                            if(subSubPrior == 2) {
                                exceptSet.push_back({subSubKey, Util::unescape(subSubValue)});
                            }
                        }
                    }
                }
                break;
            }
        }

        conf.exceptSet = exceptSet;
        return conf;
    }

} // namespace Dyno::Util