#ifndef RESOURCEJSONPARSER_HPP
#define RESOURCEJSONPARSER_HPP

#include "JSONParser.hpp"
#include "Resource.hpp"
#include "utils.hpp"

#include <iostream>

class ResourceJSONParser : public JSONParser<Resource> {
private:
    
    
    
public:
    
    explicit ResourceJSONParser(std::string_view filename) : JSONParser<Resource>(filename) {}
    
    void read_one(Resource & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    void write_one(const Resource & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    
    void replace_one(const Resource & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    
    void read_all(std::unordered_set<Resource> & set) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    void write_all(const std::unordered_set<Resource> & set) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
};

#endif
