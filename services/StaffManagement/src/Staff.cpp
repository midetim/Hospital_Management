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
    os << ansi::bcyan
       << "Staff ID: "
       << s.staff_id
       << ansi::reset
       << '\n';

    os << "  Name       : "
       << ansi::bwhite
       << s.staff_name
       << ansi::reset
       << '\n';

    os << "  Sex        : "
       << ansi::bmagenta
       << person::sexToString(s.staff_sex)
       << ansi::reset
       << '\n';

    os << "  Salary     : $"
       << ansi::byellow
       << s.staff_salary
       << ansi::reset
       << '\n';

    os << "  Room ID    : "
       << ansi::bblue
       << s.room_id
       << ansi::reset
       << '\n';

    os << "  Position   : "
       << ansi::bgreen
       << staff::position_to_string(s.staff_position)
       << ansi::reset
       << '\n';

    os << "  Clearance  : "
       << ansi::bred
       << staff::clearance_to_string(s.staff_clearance)
       << ansi::reset
       << '\n';

    return os;
}


std::unique_ptr<Staff> Staff::clone() const {
    std::unique_ptr<Staff> ptr = std::make_unique<Staff>();
    ptr->setName(this->getName());
    ptr->setSex(this->getSex());
    ptr->setStaffId(this->getStaffId());
    ptr->setRoomId(this->getRoomId());
    ptr->setSalary(this->getSalary());
    ptr->setPosition(this->getPosition());
    ptr->setClearance(this->getClearance());
    
    // Copy schedule
    for (const time_util::Shift & shift : this->view_schedule()->getFrom(time_util::timestamp_to_date(time_util::times::max))) {
        ptr->access_schedule()->addToSchedule(shift);
    }
    
    return ptr;
}
