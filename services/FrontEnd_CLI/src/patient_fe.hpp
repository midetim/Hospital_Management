#ifndef PATIENT_FE_HPP
#define PATIENT_FE_HPP

#include <string>

// Forward declarations
class PatientManagementClient;
namespace person  { enum class Sex; }
namespace patient { enum class Condition; }

namespace patient_front_end {
    // Input parsers
    person::Sex parseSexInput(std::string s);
    patient::Condition parseConditionInput(std::string s);
        
    // User input polling functions
    void admit(const PatientManagementClient & ref);
    void discharge(const PatientManagementClient & ref);
    void transfer(const PatientManagementClient & ref);
    void quarantine(const PatientManagementClient & ref);
    void info(const PatientManagementClient & ref);
    void update(const PatientManagementClient & ref);
    void menu(const PatientManagementClient & ref);

}

#endif
