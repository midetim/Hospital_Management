#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP

#include "json.hpp"

#include <cstdint>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>


using nlohmann::json;

template<typename T>
class JSONParser {
protected:
    std::string filename;
    
    inline void array_to_set(const json & j, std::unordered_set<uint64_t> & s) {
        if (!j.is_array()) { return; }
        s.clear();
        for (const json & obj : j) {
            s.insert(obj.get<uint64_t>());
        }
    }
    
    inline void array_to_set(const json & j, std::unordered_set<uint32_t> & s) {
        if (!j.is_array()) { return; }
        s.clear();
        for (const json & obj : j) {
            s.insert(obj.get<uint32_t>());
        }
    }
    
    inline void set_to_array(const std::unordered_set<uint64_t> & s, nlohmann::json & j) {
        j = nlohmann::json::array();
        for (uint64_t id : s) {
            j.push_back(id);
        }
    }
    
    inline void set_to_array(const std::unordered_set<uint32_t> & s, nlohmann::json & j) {
        j = nlohmann::json::array();
        for (uint32_t id : s) {
            j.push_back(id);
        }
    }
    
    json open_file() {
        std::ifstream file(filename.data());
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + std::string(filename));
        }
        
        json j;
        
        try {
            file >> j;
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            throw;
        } 
        
        return j;
    }
    
public:
    
    explicit JSONParser(std::string filename) : filename(filename) {}
    virtual ~JSONParser() = default;
    
    virtual void read_one(T & obj) = 0;
    virtual void write_one(const T & obj) = 0; // Add new
    
    virtual void replace_one(const T & obj) = 0; // Change old
    virtual void remove_one(uint64_t id) = 0; // Remove empty
    
    virtual void read_all(std::vector<std::unique_ptr<T>> & v) = 0;
    virtual void write_all(const std::vector<std::unique_ptr<T>> & v) = 0;
};

#endif

