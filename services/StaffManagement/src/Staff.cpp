#include "Staff.hpp"

using namespace person;
using namespace staff;


/* ******************************************************************** */
/* ********************** Staff Constructors ************************** */
/* ******************************************************************** */

Staff::Staff() : staff_schedule(std::make_unique<Schedule>()) {}

Staff::Staff(const Name & name, Sex sex, float salary, Position position, Clearance clearance) : Staff() {
    this->staff_name = name;
    this->staff_sex = sex;
    this->staff_salary = salary;
    this->staff_position = position;
    this->staff_clearance = clearance;
}

Staff::Staff(const person::Name & name, person::Sex sex) : Staff(name, sex, 0.0f, Position::None, Clearance::None) {}

/* ******************************************************************** */
/* ***************************** Other ******************************** */
/* ******************************************************************** */

bool Staff::operator==(const Staff & other) const {
    return  this->staff_name.first == other.staff_name.first &&
            this->staff_name.middle == other.staff_name.middle &&
            this->staff_name.last == other.staff_name.last &&
            this->staff_sex == other.staff_sex;
}

std::ostream & operator<<(std::ostream & os, const Staff & s) {
    os << "Staff [ID: "   << s.staff_id
       << ", Name: "      << s.staff_name
       << ", Sex: "       << sexToString(s.staff_sex)
       << ", Salary: $"   << s.staff_salary
       << ", Room ID: "   << s.room_id
       << ", Position: "  << staff::position_to_string(s.staff_position)
       << ", Clearance: " << static_cast<int>(s.staff_clearance)
       << "]";

    return os;
}
