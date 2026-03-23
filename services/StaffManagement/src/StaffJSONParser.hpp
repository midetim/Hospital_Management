#ifndef STAFFJSONPARSER_HPP
#define STAFFJSONPARSER_HPP

#include "JSONParser.hpp"
#include "Staff.hpp"
#include "utils.hpp"

#include <iostream>

class StaffJSONParser : public JSONParser<Staff> {
public:
    
    explicit StaffJSONParser(std::string_view filename) : JSONParser<Staff>(filename) {}
    
    void read_one(Staff & s) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    void write_one(const Staff & s) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    
    void replace_one(const Staff & s) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    
    void read_all(std::unordered_set<Staff> & set) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    void write_all(const std::unordered_set<Staff> & set) override {
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

