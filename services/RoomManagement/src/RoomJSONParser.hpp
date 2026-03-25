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
    inline constexpr std::string_view qtine     = "room-quarantined";
}

class RoomJSONParser : public JSONParser<Room> {
private:
    
    inline void from_json(const json & j, Room & r) {
        r.setRoomType(room::stringToRoomType(j.at(label::type).get<std::string>()));
        r.setRoomId(j.at(label::id).get<uint32_t>());
        r.setRoomCapacity(j.at(label::maximum).get<uint32_t>());
        r.setCurrentCapacity(j.at(label::current).get<uint32_t>());
        
        array_to_set(j.at(label::staff), r.getAssignedStaff());
        array_to_set(j.at(label::patients), r.getAssignedPatients());
        array_to_set(j.at(label::resources), r.getAssignedResources());
        
        bool quarantined = j.at(label::qtine).get<bool>();
        if (quarantined) { r.quarantineRoom(); }
    }

    inline void to_json(const Room & r, json & j) {
        j = nlohmann::json{
            {label::type, room::roomTypeToString(r.getRoomType())},
            {label::id, r.getRoomId()},
            {label::maximum, r.getRoomCapacity()},
            {label::current, r.getCurrentCapacity()},
            {label::qtine, r.getQuarantineStatus()},
        };
        
        set_to_array(r.getAssignedStaff(), j[label::staff]);
        set_to_array(r.getAssignedPatients(), j[label::patients]);
        set_to_array(r.getAssignedResources(), j[label::resources]);
    }

public:
    explicit RoomJSONParser(std::string filename) : JSONParser<Room>(filename) {}
    
    void read_one(Room & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        Room temp;
        for (const json & obj : j) {
            from_json(obj, temp);
            
            if (r == temp) {
                r = temp;
                return;
            } else { continue; }
            
        }
        
        std::cout << Utils::timestamp() << ansi::red << "Unable to find the Room: " << ansi::reset << r << std::endl;
    }
    
    void write_one(const Room & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        json new_r;
        to_json(r, new_r);
        j.push_back(new_r);
        
        std::ofstream file(filename.data());
        if (!file.is_open()) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Failed to open JSON file for writing." << ansi::reset << std::endl;
            return;
        }

        file << j.dump(4);
    }
    
    
    void replace_one(const Room & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        bool found = false;
        for (json & obj : j) {
            Room temp;
            from_json(obj, temp);
            if (temp == r) { // Room ids match
                to_json(r, obj); // Overwrite this element
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Room not found in JSON file." << ansi::reset << std::endl;
            return;
        }

        // Write updated array back to file
        std::ofstream file(filename.data());
        if (!file.is_open()) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Failed to open JSON file for writing." << ansi::reset << std::endl;
            return;
        }

        file << j.dump(4);
        
    }
    
    // Unable to remove rooms
    void remove_one(uint64_t id) override { return; }
    
    void read_all(std::vector<std::unique_ptr<Room>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        v.clear();
        
        for (const json & obj : j) {
            std::unique_ptr<Room> ptr = std::make_unique<Room>();
            from_json(obj, * ptr);
            v.push_back(std::move(ptr));
        }
        
        
    }
    
    void write_all(const std::vector<std::unique_ptr<Room>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        j.clear();
        
        for (const std::unique_ptr<Room> & ptr : v) {
            json obj;
            to_json(* ptr, obj);
            j.push_back(obj);
        }
        
        // Write updated array back to file
        std::ofstream file(filename.data());
        if (!file.is_open()) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Failed to open JSON file for writing." << ansi::reset << std::endl;
            return;
        }

        file << j.dump(4);
    }
    
};

#endif
