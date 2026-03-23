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
            .first =  j[label::first],
            .middle = j[label::middle],
            .last =   j[label::last]
        };
        p.setPatientName(patient_name);
        p.setPatientSex(person::stringToSex(j[label::sex]));
        p.setPatientCondition(patient::stringToCondition(j[label::cond]));
        p.setPatientId(j[label::id]);
        p.setRoomId(j[label::room]);
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
    explicit PatientJSONParser(std::string_view filename) : JSONParser<Patient>(filename) {}
    
    void read_one(Patient & p) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        Patient old_p = p;
        for (const json & obj : j) {
            from_json(obj, p);
            
            bool name_match = old_p == p;
            bool id_match = old_p.getPatientId() == p.getPatientId();
            if (id_match || name_match) {
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
    
    void read_all(std::unordered_set<Patient> & set) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        Patient p;
        for (const json & obj : j) {
            from_json(obj, p);
            set.emplace(p);
        }
    }
    
    void write_all(const std::unordered_set<Patient> & set) override {
        json j;
        try {
            j = open_file();
        } catch (...) {
            std::cout << Utils::timestamp() << ansi::red << "Unable to access JSON file." << ansi::reset << std::endl;
            return;
        }
        
        j.clear();
        
        json obj;
        for (const Patient & p : set) {
            to_json(p, obj);
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
