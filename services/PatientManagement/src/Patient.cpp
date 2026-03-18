#include "Patient.hpp"

using namespace core;
using namespace person;
using namespace patient;

// Public
Patient::Patient(const Name & name, Sex sex = Sex::Unknown) // Master Constuctor
    : patient_name(name), patient_sex(sex) {
        
    // Special handling for default female name
    if (sex == Sex::Female && patient_name.first == "John") {
        patient_name.first = "Jane";
    }
        
}

// Default constructor
Patient::Patient()
    : Patient(Name{ "John", "", "Doe" }, Sex::Unknown) {}

// Sex-only constructor
Patient::Patient(Sex sex)
    : Patient(Name{ "John", "", "Doe" }, sex) {}

// Name-only constructor
Patient::Patient(const Name & name) 
    : Patient(name, Sex::Unknown) {}

Patient::~Patient() {
    
    
}

bool Patient::operator==(const Patient & other) const {
    return  this->patient_name.first == other.patient_name.first &&
            this->patient_name.middle == other.patient_name.middle &&
            this->patient_name.last == other.patient_name.last &&
            this->patient_sex == other.patient_sex;
}

ReturnCode Patient::updateName(Name & name) {
    Name temp = this->patient_name; // Get a temp name equal to the patients current name
    if (temp.first == name.first && temp.middle == name.middle && temp.last == name.last) {
        return ReturnCode::WARNING; // Gives a warning if the name is not changed
    }
    this->patient_name = name;
    return ReturnCode::SUCCESS;
}



std::ostream & operator<<(std::ostream& os, const Patient & p) {
    const Name& n = p.getPatientName();
    Sex s = p.getPatientSex();
    Condition c = p.getPatientCondition();
    uint32_t rid = p.getRoomId();

    os << ansi::bcyan
       << "Patient ID: "
       << p.getPatientId()
       << ansi::reset
       << '\n';

    os << "  Name     : "
       << ansi::bwhite
       << n.first << " " << n.middle << " " << n.last
       << ansi::reset
       << '\n';

    os << "  Sex      : "
       << (s == Sex::Male ? ansi::bblue : ansi::bred)
       << sexToString(s)
       << ansi::reset
       << '\n';

    os << "  Condition: "
       << (c == Condition::Critical ? ansi::bred : ansi::bgreen)
       << conditionToString(c)
       << ansi::reset
       << '\n';

    os << "  Room ID  : "
       << ansi::bmagenta
       << rid
       << ansi::reset
       << '\n';

    return os;
}
