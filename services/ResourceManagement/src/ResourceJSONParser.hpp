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

// expr printf("%s\n", j.dump(4, ' ', true, nlohmann::detail::error_handler_t::strict).c_str())

class ResourceJSONParser : public JSONParser<Resource> {
private:
    
    inline void from_json(const json & j, time_util::Shift & s) {
        time_util::Timestamp start_ts{j.at(label::shift::start).get<uint64_t>()};
        time_util::Timestamp end_ts{j.at(label::shift::end).get<uint64_t>()};
        s = time_util::Shift(start_ts, end_ts, j.at(label::shift::room).get<uint32_t>());
    }
    
    inline void to_json(json & j, const time_util::Shift & s) {
        j = json{
            {label::shift::start, s.shift_start.time},
            {label::shift::end,   s.shift_end.time},
            {label::shift::dur,   s.duration},
            {label::shift::room,  s.room_id}
        };
    }
    
    inline void from_json(const json & j, Resource & r) {
        r.setResourceId(j.at(label::id).get<uint64_t>());
        r.setType(j.at(label::type).get<std::string>());
        
        auto it = resource::types.find(r.getType());
        if (it == resource::types.end()) {
            return; // Idk
        } else if (it->second == resource::machine) {
            r.setMachine(true);
        } else if (it->second == resource::consumable) {
            r.setConsumable(true);
        } else {
            return;
        }
        
        if (r.getConsumable()) {
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
    
    inline void to_json(json & j, const Resource & r) {
        j[label::id]    = r.getResourceId();
        j[label::room]  = r.getRoomId();
        j[label::stock] = r.getStock();
        j[label::type] = r.getType();
    
        if (r.getConsumable()) {
            j[label::sched] = json::array();
        } else {
            json schedule = json::array();
            
            std::set<time_util::Shift> shifts =  r.view_schedule()->getFrom(time_util::timestamp_to_date(time_util::times::max));
            
            for (const auto& shift : shifts) {
                json shift_json;
                to_json(shift_json, shift);
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
        
        for (const json & obj : j) {
            Resource temp;
            from_json(obj, temp);
            if (temp.getResourceId() == r.getResourceId()) {
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
        to_json(new_r, r);
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
                to_json(obj, r);
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
            to_json(obj, * ptr);
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
