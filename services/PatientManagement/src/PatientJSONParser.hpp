#ifndef PATIENTJSONPARSER_HPP
#define PATIENTJSONPARSER_HPP

#include "JSONParser.hpp"
#include "Patient.hpp"
#include "utils.hpp"

#include <iostream>

namespace label {
    inline constexpr std::string_view first = "name-first";
    inline constexpr std::string_view middle = "name-middle";
    inline constexpr std::string_view last = "name-last";
    inline constexpr std::string_view sex = "sex";
    inline constexpr std::string_view cond = "condition";
    inline constexpr std::string_view id = "id";
    inline constexpr std::string_view room = "room";
}


class PatientJSONParser : public JSONParser<Patient> {
private:
    
    inline void from_json(const nlohmann::json & j, Patient & p) {
        person::Name patient_name = {
            .first =  j.at(label::first).get<std::string>(),
            .middle = j.at(label::middle).get<std::string>(),
            .last =   j.at(label::last).get<std::string>()
        };
        p.setPatientName(patient_name);
        p.setPatientSex(person::stringToSex(j.at(label::sex).get<std::string>()));
        p.setPatientCondition(patient::stringToCondition(j.at(label::cond).get<std::string>()));
        p.setPatientId(j.at(label::id).get<uint64_t>());
        p.setRoomId(j.at(label::room).get<uint32_t>());
    }
    
    inline void to_json(const Patient & p, nlohmann::json & j) {
        j = nlohmann::json{
            {label::first,  p.getPatientName().first},
            {label::middle, p.getPatientName().middle},
            {label::last,   p.getPatientName().last},
            {label::sex,    person::sexToString(p.getPatientSex())},
            {label::cond,   patient::conditionToString(p.getPatientCondition())},
            {label::id,     p.getPatientId()},
            {label::room,   p.getRoomId()},
        };
    }
    
public:
    explicit PatientJSONParser(std::string filename) : JSONParser<Patient>(filename) {}
    
    void read_one(Patient & p) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        Patient temp;
        for (const json & obj : j) {
            from_json(obj, temp);
            
            bool name_match = temp == p;
            bool id_match = temp.getPatientId() == p.getPatientId();
            if (id_match || name_match) {
                p = temp;
                return;
            } else { continue; }
        }
        
        std::cout << Utils::timestamp() << ansi::red << "Unable to find the patient: " << ansi::reset << p << std::endl;
    }
    
    void write_one(const Patient & p) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        json new_p;
        to_json(p, new_p);
        j.push_back(new_p);
        
        std::ofstream file(filename.data());
        if (!file.is_open()) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Failed to open JSON file for writing." << ansi::reset << std::endl;
            return;
        }

        file << j.dump(4);
    }
    
    void replace_one(const Patient & p) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        bool found = false;
        for (json & obj : j) {
            Patient temp;
            from_json(obj, temp);
            if (p == temp || p.getPatientId() == temp.getPatientId()) {
                to_json(p, obj);  // overwrite this element
                found = true;
                break;
            }
        }

        if (!found) {
            std::cout << Utils::timestamp() << ansi::red
                      << "Patient not found in JSON file." << ansi::reset << std::endl;
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
            Patient temp;
            from_json(* it, temp);

            if (temp.getPatientId() == id) {
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
    
    void read_all(std::vector<std::unique_ptr<Patient>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        v.clear(); // Empty the set to prep for writing to
        
        for (const json & obj : j) {
            std::unique_ptr<Patient> ptr = std::make_unique<Patient>();
            from_json(obj, * ptr);
            v.push_back(std::move(ptr));
        }
    }
    
    void write_all(const std::vector<std::unique_ptr<Patient>> & v) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        j.clear(); // Empty the json to prep for writing to
        
        for (const std::unique_ptr<Patient> & ptr : v) {
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
