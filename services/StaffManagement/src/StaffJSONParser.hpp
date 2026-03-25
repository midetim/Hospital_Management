#ifndef STAFFJSONPARSER_HPP
#define STAFFJSONPARSER_HPP

#include "JSONParser.hpp"
#include "Staff.hpp"
#include "time_utils.hpp"

#include <iostream>

namespace label {
    inline constexpr std::string_view first     = "name-first";
    inline constexpr std::string_view middle    = "name-middle";
    inline constexpr std::string_view last      = "name-last";
    inline constexpr std::string_view sex       = "sex";
    inline constexpr std::string_view id        = "id";
    inline constexpr std::string_view room      = "room";
    inline constexpr std::string_view salary    = "salary";
    inline constexpr std::string_view position  = "position";
    inline constexpr std::string_view clearance = "clearance";
    inline constexpr std::string_view schedule  = "schedule";

    namespace shift {
        inline constexpr std::string_view start = "shift-start";
        inline constexpr std::string_view end = "shift-end";
        inline constexpr std::string_view dur = "shift-duration";
        inline constexpr std::string_view room = "shift-room";
    }
}

class StaffJSONParser : public JSONParser<Staff> {
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
    
    inline void from_json(const json & j, Staff & s) {
        person::Name n = {
            .first  = j.at(label::first).get<std::string>(),
            .middle = j.at(label::middle).get<std::string>(),
            .last   = j.at(label::last).get<std::string>()
        };
        s.setName(n);
        s.setSex(person::stringToSex(j.at(label::sex).get<std::string>()));
        s.setStaffId(j.at(label::id).get<uint64_t>());
        s.setRoomId(j.at(label::room).get<uint32_t>());
        s.setSalary(j.at(label::salary).get<float>());
        s.setPosition(staff::string_to_position(j.at(label::position).get<std::string>()));
        s.setClearance(staff::string_to_clearance(j.at(label::clearance).get<std::string>()));
        
        const json & schedule = j.at(label::schedule);
        s.access_schedule()->clear();

        for (const json & shift_json : schedule) {
            time_util::Shift shift;
            from_json(shift_json, shift);
            s.access_schedule()->addToSchedule(shift);
        }
    }
    
    inline void to_json(const Staff & s, json & j) {
        j = json {
            {label::first, s.getName().first},
            {label::middle, s.getName().middle},
            {label::last, s.getName().last},
            {label::sex, person::sexToString(s.getSex())},
            {label::id, s.getStaffId()},
            {label::room, s.getRoomId()},
            {label::salary, s.getSalary()},
            {label::position, staff::position_to_string(s.getPosition())},
            {label::clearance, staff::clearance_to_string(s.getClearance())}
        };
        
        json schedule = json::array();
        
        std::set<time_util::Shift> shifts = s.view_schedule()->getFrom(time_util::timestamp_to_date(time_util::times::max));

        for (const time_util::Shift & shift : shifts) {
            json shift_json;
            to_json(shift, shift_json);
            schedule.push_back(shift_json);
        }
        
        j[label::schedule] = schedule;
    }
    
    
public:
    
    explicit StaffJSONParser(std::string filename) : JSONParser<Staff>(filename) {}
    
    void read_one(Staff & s) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        Staff temp;
        for (const json & obj : j) {
            from_json(obj, temp);
            
            bool name_match = temp == s;
            bool id_match = temp.getStaffId() == s.getStaffId();
            if (id_match || name_match) {
                s = std::move(temp);
                return;
            } else { continue; }
        }
        
        std::cout << Utils::timestamp() << ansi::red << "Unable to find the staff: " << ansi::reset << s << std::endl;
    }
    
    void write_one(const Staff & s) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        json new_s;
        to_json(s, new_s);
        j.push_back(new_s);
        
        std::ofstream file(filename.data());
        if (!file.is_open()) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Failed to open JSON file for writing." << ansi::reset << std::endl;
            return;
        }

        file << j.dump(4);
    }
    
    
    void replace_one(const Staff & s) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        bool found = false;
        for (json & obj : j) {
            Staff temp;
            from_json(obj, temp);
            if (s == temp || s.getStaffId() == temp.getStaffId()) {
                to_json(s, obj);
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Staff not found in JSON file." << ansi::reset << std::endl;
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
            Staff temp;
            from_json(* it, temp);

            if (temp.getStaffId() == id) {
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
    
    
    void read_all(std::vector<std::unique_ptr<Staff>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        v.clear();
        
        for (const json & obj : j) {
            std::unique_ptr<Staff> ptr = std::make_unique<Staff>();
            from_json(obj, * ptr);
            v.push_back(std::move(ptr));
        }
    }
    
    void write_all(const std::vector<std::unique_ptr<Staff>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        j.clear();
        
        for (const std::unique_ptr<Staff> & ptr : v) {
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

