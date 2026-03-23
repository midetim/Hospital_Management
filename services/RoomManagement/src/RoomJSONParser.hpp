#ifndef ROOMJSONPARSER_HPP
#define ROOMJSONPARSER_HPP

#include "JSONParser.hpp"
#include "Room.hpp"
#include "utils.hpp"

#include <iostream>

namespace label {
    inline constexpr std::string_view type      = "type";
    inline constexpr std::string_view id        = "id";
    inline constexpr std::string_view maximum   = "maximum-capacity";
    inline constexpr std::string_view current   = "current-capacity";
    inline constexpr std::string_view staff     = "assigned-staff";
    inline constexpr std::string_view patients  = "assigned-patients";
    inline constexpr std::string_view resources = "assigned-resources";
}

class RoomJSONParser : public JSONParser<Room> {
private:
    
    inline void from_json(const json & j, Room & r) {
        r.setRoomType(room::stringToRoomType(j[label::type]));
        r.setRoomId(j[label::id]);
        r.setRoomCapacity(j[label::maximum]);
        r.setCurrentCapacity(j[label::current]);
        
        array_to_set(j[label::staff], r.getAssignedStaff());
        array_to_set(j[label::patients], r.getAssignedPatients());
        array_to_set(j[label::resources], r.getAssignedResources());
    }

    inline void to_json(const Room & r, json & j) {
        j = nlohmann::json{
            {label::type, room::resourceTypeToString(r.getResourceType())},
            {label::id, r.getRoomId()},
            {label::maximum, r.getRoomCapacity()},
            {label::current, r.getCurrentCapacity()},
        };
        
        set_to_array(r.getAssignedStaff(), j[label::staff]);
        set_to_array(r.getAssignedPatients(), j[label::patients]);
        set_to_array(r.getAssignedResources(), j[label::resources]);
    }

public:
    explicit RoomJSONParser(std::string_view filename) : JSONParser<Room>(filename) {}
    
    void read_one(Room & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    void write_one(const Room & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    
    void replace_one(const Room & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    
    void read_all(std::unordered_set<Room> & set) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
    }
    
    void write_all(const std::unordered_set<Room> & set) override {
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
