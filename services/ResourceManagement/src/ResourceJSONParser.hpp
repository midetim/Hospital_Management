#ifndef RESOURCEJSONPARSER_HPP
#define RESOURCEJSONPARSER_HPP

#include "JSONParser.hpp"
#include "Resource.hpp"
#include "time_utils.hpp"

#include <iostream>

namespace label {
    inline constexpr std::string_view id    = "id";
    inline constexpr std::string_view room  = "room";
    inline constexpr std::string_view type  = "type";
    inline constexpr std::string_view stock = "stock";
    inline constexpr std::string_view sched = "schedule";

    namespace shift {
        inline constexpr std::string_view start = "shift-start";
        inline constexpr std::string_view end   = "shift-end";
        inline constexpr std::string_view dur   = "shift-duration";
        inline constexpr std::string_view room  = "shift-room";
    }
}


class ResourceJSONParser : public JSONParser<Resource> {
private:
    
    inline void from_json(const json & j, time_util::Shift & s) {
        time_util::Timestamp start_ts{j.at(label::shift::start).get<uint64_t>()};
        time_util::Timestamp end_ts{j.at(label::shift::end).get<uint64_t>()};
        s = time_util::Shift(start_ts, end_ts, j.at(label::shift::room).get<uint32_t>());
    }
    
    inline void to_json(const time_util::Shift & s, json & j) {
        j = json{
            {label::shift::start, s.shift_start.time},
            {label::shift::end,   s.shift_end.time},
            {label::shift::dur,   s.duration},
            {label::shift::room,  s.room_id}
        };
    }
    
    inline void from_json(const json & j, Resource & r) {
        r.setResourceId(j.at(label::id).get<uint64_t>());
        r.setStock(j.at(label::stock).get<uint32_t>());
        r.setResourceType(resource::stringToResourceType(j.at(label::type).get<std::string>()));
        
        if (resource::isConsumable(r.getResourceType())) {
            r.setStock(j.at(label::stock).get<uint32_t>());
            r.setRoomId(room::none);
        } else {
            r.setRoomId(j.at(label::room).get<uint32_t>());
            r.setStock(0);
            
            const json & schedule = j.at(label::sched);
            r.access_schedule()->clear();

            for (const json & shift_json : schedule) {
                time_util::Shift shift;
                from_json(shift_json, shift);
                r.access_schedule()->addToSchedule(shift);
            }
        }
    }
    
    inline void to_json(const Resource & r, json & j) {
        j = json{
            {label::id, r.getResourceId()},
            {label::room, r.getRoomId()},
            {label::type, resource::resourceTypeToString(r.getResourceType())},
            {label::stock, r.getStock()}
        };
        
        if (resource::isConsumable(r.getResourceType())) {
            j[label::sched] = json::array();
        } else {
            json schedule = json::array();
            
            std::set<time_util::Shift> shifts =  r.view_schedule()->getFrom(time_util::timestamp_to_date(time_util::times::max));
            
            for (const auto& shift : shifts) {
                json shift_json;
                to_json(shift, shift_json);
                schedule.push_back(shift_json);
            }
            
            j[label::sched] = schedule;
            
        }
    }
    
public:
    
    explicit ResourceJSONParser(std::string filename) : JSONParser<Resource>(filename) {}
    
    void read_one(Resource & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        Resource temp;
        for (const json & obj : j) {
            from_json(obj, temp);
            if (temp == r) {
                r = std::move(temp);
                return;
            } else { continue; }
        }
        
        std::cout << Utils::timestamp() << ansi::red << "Unable to find the resource: " << ansi::reset << r << std::endl;
    }
    
    void write_one(const Resource & r) override {
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
    
    
    void replace_one(const Resource & r) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        bool found = false;
        for (json & obj : j) {
            Resource temp;
            from_json(obj, temp);
            if (r == temp) {
                to_json(r, obj);
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Resource not found in JSON file." << ansi::reset << std::endl;
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
    
    void remove_one(uint64_t id) override {
        if (id == 0) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Invalid patient ID for removal." << ansi::reset << std::endl;
            return;
        }

        json j;
        try {
            j = open_file(); // read the existing JSON array
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }

        bool found = false;
        for (auto it = j.begin(); it != j.end(); ++it) {
            Resource temp;
            from_json(* it, temp);

            if (temp.getResourceId() == id) {
                j.erase(it); // remove the patient
                found = true;
                break;
            }
        }

        if (!found) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Patient ID " << id << " not found in JSON file." << ansi::reset << std::endl;
            return;
        }

        std::ofstream file(filename.data());
        if (!file.is_open()) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Failed to open JSON file for writing." << ansi::reset << std::endl;
            return;
        }

        file << j.dump(4); // write the updated array back
    }
    
    void read_all(std::vector<std::unique_ptr<Resource>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        v.clear();
        
        for (const json & obj : j) {
            std::unique_ptr<Resource> ptr = std::make_unique<Resource>();
            from_json(obj, * ptr);
            v.push_back(std::move(ptr));
        }
        
    }
    
    void write_all(const std::vector<std::unique_ptr<Resource>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        j.clear();
        
        for (const std::unique_ptr<Resource> & ptr : v) {
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
